#include "map_renderer.h"

#include <optional>

using namespace renderer;
using namespace std;
using namespace svg;

//----------------------SphereProjector----------------------
template<typename PointInputIt>
inline SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
	double max_width, double max_height, double padding) : padding_(padding)
{
	// Если точки поверхности сферы не заданы, вычислять нечего
	if (points_begin == points_end) {
		return;
	}
	// Находим точки с минимальной и максимальной долготой
	const auto[left_it, right_it] = std::minmax_element(
		points_begin, points_end,
		[](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
	min_lon_ = left_it->lng;
	const double max_lon = right_it->lng;

	// Находим точки с минимальной и максимальной широтой
	const auto[bottom_it, top_it] = std::minmax_element(
		points_begin, points_end,
		[](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
	const double min_lat = bottom_it->lat;
	max_lat_ = top_it->lat;

	// Вычисляем коэффициент масштабирования вдоль координаты x
	std::optional<double> width_zoom;
	if (!IsZero(max_lon - min_lon_)) {
		width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
	}

	// Вычисляем коэффициент масштабирования вдоль координаты y
	std::optional<double> height_zoom;
	if (!IsZero(max_lat_ - min_lat)) {
		height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
	}

	if (width_zoom && height_zoom) {
		// Коэффициенты масштабирования по ширине и высоте ненулевые,
		// берём минимальный из них
		zoom_coeff_ = std::min(*width_zoom, *height_zoom);
	}
	else if (width_zoom) {
		// Коэффициент масштабирования по ширине ненулевой, используем его
		zoom_coeff_ = *width_zoom;
	}
	else if (height_zoom) {
		// Коэффициент масштабирования по высоте ненулевой, используем его
		zoom_coeff_ = *height_zoom;
	}
}

svg::Point renderer::SphereProjector::operator()(geo::Coordinates coords) const
{
	return {
		(coords.lng - min_lon_) * zoom_coeff_ + padding_,
		(max_lat_ - coords.lat) * zoom_coeff_ + padding_
	};
}

//----------------------MapRenderer----------------------
std::unordered_set<geo::Coordinates, CoordinatesHasher> MapRenderer::GetCoordinates() const
{
	std::unordered_set<geo::Coordinates, CoordinatesHasher> result;
	//получение всех координат остановок
	for (std::string_view bus_name : catalogue_)
	{
		std::optional<const domain::Bus*> bus_finded = catalogue_.GetBusInfo(bus_name);

		if (!bus_finded) {
			throw logic_error("Catalog error. No Bus info"s + static_cast<std::string> (bus_name));
		}

		for (const domain::Stop* stop : (*bus_finded)->stops) {
			result.insert(stop->coordinate);
		}
	}
	return result;
}

void renderer::MapRenderer::Render(std::ostream & out)
{
	auto coords = GetCoordinates();
	SphereProjector projector(coords.begin(), coords.end(), settings_.width, settings_.height, settings_.padding);

	svg::Document document;

	set<string_view> stops_in_buses = RenderBuses(projector, document);
	RenderStops(projector, document, stops_in_buses);
	document.Render(out);
}

pair<unique_ptr<Text>, unique_ptr<Text>> MapRenderer::AddBusLabels(SphereProjector& project,
	int index_color, const domain::Stop* stop, string_view name)
{
	Text bus_name_underlabel, bus_name_label;

	bus_name_underlabel.SetData(static_cast<string>(name)).SetPosition(project(stop->coordinate))
		.SetOffset(settings_.bus_label_offset).SetFontSize(settings_.bus_label_font_size)
		.SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetStrokeWidth(settings_.underlayer_width)
		.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color)
		.SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND);

	bus_name_label.SetData(static_cast<string>(name)).SetPosition(project(stop->coordinate))
		.SetOffset(settings_.bus_label_offset).SetFontSize(settings_.bus_label_font_size)
		.SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetFillColor(settings_.color_palette[index_color]);

	return { make_unique<Text>(bus_name_underlabel), make_unique<Text>(bus_name_label) };
}

set<string_view> MapRenderer::RenderBuses(SphereProjector& projector, Document& doc_to_render)
{
	int index_color = 0;
	int color_counts = settings_.color_palette.size();

	vector<unique_ptr<Object>> bus_lines;
	vector<unique_ptr<Object>> bus_labels;

	bus_lines.reserve(catalogue_.size());
	bus_labels.reserve(bus_lines.capacity() * 4);
	set<string_view> stops_in_buses;

	for (string_view bus_name : catalogue_) {

		index_color %= color_counts;
		//
		const domain::Bus* bus = *catalogue_.GetBusInfo(bus_name);
		if (bus->stops.empty()) {
			continue;
		}

		unique_ptr<Polyline> line = make_unique<Polyline>(Polyline().SetFillColor("none"s)
			.SetStrokeColor(settings_.color_palette[index_color]).SetStrokeWidth(settings_.line_width)
			.SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND));

		unique_ptr<Text> bus_label_start, bus_underlabel_start, bus_label_finish, bus_underlabel_finish;

		tie(bus_underlabel_start, bus_label_start) = AddBusLabels(projector, index_color, bus->stops.front(), bus_name);

		if (!bus->route_type && (bus->stops.front() != bus->stops[bus->stops.size() / 2])) {
			tie(bus_underlabel_finish, bus_label_finish) = AddBusLabels(projector, index_color, bus->stops[bus->stops.size() / 2], bus_name);
		}

		for (const domain::Stop* stop : bus->stops) {
			line->AddPoint(projector(stop->coordinate));
			stops_in_buses.insert(stop->name);
		}

		bus_lines.push_back(move(line));
		bus_labels.push_back(move(bus_underlabel_start));
		bus_labels.push_back(move(bus_label_start));

		if (bus_label_finish && bus_underlabel_finish) {
			bus_labels.push_back(move(bus_underlabel_finish));
			bus_labels.push_back(move(bus_label_finish));
		}
		++index_color;
	}

	for (auto& pointer : bus_lines) {
		doc_to_render.AddPtr(move(pointer));
	}
	for (auto& pointer : bus_labels) {
		doc_to_render.AddPtr(move(pointer));
	}
	return stops_in_buses;
}

void MapRenderer::RenderStops(SphereProjector& projector, svg::Document& doc_to_render, set<string_view> stops_in_buses) {
	vector<unique_ptr<Circle>> stop_points;
	vector<unique_ptr<Text>> stop_labels;
	stop_points.reserve(stops_in_buses.size());
	stop_labels.reserve(stops_in_buses.size() * 2);

	for (string_view stop_name : stops_in_buses) {
		const domain::Stop* stop = *(catalogue_.GetStopInfo(stop_name));
		Point coords = projector(stop->coordinate);

		unique_ptr<Circle> stop_point = make_unique<Circle>(Circle().SetCenter(coords)
			.SetRadius(settings_.stop_radius)
			.SetFillColor("white"s));

		unique_ptr<Text> stop_underlabel = make_unique<Text>(Text().SetFillColor(settings_.underlayer_color)
			.SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width)
			.SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND)
			.SetPosition(coords).SetOffset(settings_.stop_label_offset)
			.SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s)
			.SetData(static_cast<string>(stop_name))
			);

		unique_ptr<Text> stop_label = make_unique<Text>(Text().SetPosition(coords)
			.SetData(static_cast<string>(stop_name)).SetOffset(settings_.stop_label_offset)
			.SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s)
			.SetFillColor("black"s));

		stop_points.push_back(move(stop_point));
		stop_labels.push_back(move(stop_underlabel));
		stop_labels.push_back(move(stop_label));
	}

	for (auto& pointer : stop_points) {
		doc_to_render.AddPtr(move(pointer));
	}
	for (auto& pointer : stop_labels) {
		doc_to_render.AddPtr(move(pointer));
	}
}