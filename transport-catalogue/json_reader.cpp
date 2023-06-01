#include "json_reader.h"
#include <sstream>

using namespace json_reader;

const std::vector<domain::BusInput> JsonReader::GetBuses() const
{
    return buses_;
}

const std::vector<domain::StopInput> JsonReader::GetStops() const
{
    return stops_;
}

const std::map<std::pair<std::string, std::string>, int> JsonReader::GetDistances() const
{
    return distances_;
}

void JsonReader::LoadDocument(std::istream &input)
{
    document_ = json::Load(input);
}

void JsonReader::ReadDocument()
{
    if (document_.GetRoot().IsNull())
        return;

    auto& it = document_.GetRoot().AsMap();

    if (it.count("base_requests"s))
    {
        ParseBase(it.at("base_requests"s));
    }
    if (it.count("stat_requests"s) && it.at("stat_requests"s).IsArray())
    {
        ParseStats(it.at("stat_requests"s));
    }
    if (it.count("render_settings"s))
    {
        ParseSettings(it.at("render_settings"s));
    }
}

void JsonReader::PrintDocument(json::Document& document_answer,
	                           json::Builder& builder,
                               const std::string& answer_map)
{
	builder.StartArray();

    for (const auto& stat : stats_) {
        if (stat.type == "Stop")
        {
            std::optional<std::set<std::string_view>> info = catalogue_.GetBusesOnStop(stat.name);
            if (!info)
            {
				ErrorMassage(builder, stat.id);
            }
            else {
				builder.StartDict().Key("buses"s).StartArray();
				for (const auto& value : *info)
                {
					builder.Value(std::string(value));
                }
				builder.EndArray();
				//
				builder.Key("request_id"s).Value(stat.id).EndDict();
            }
        }
        else if (stat.type == "Bus")
        {
            std::optional<const domain::Bus*> info = catalogue_.GetBusInfo(stat.name);
            if (!info)
            {
				ErrorMassage(builder, stat.id);
            }
            else {
				builder.StartDict()
					.Key("curvature"s).Value((*info)->curvature)
					.Key("request_id"s).Value(stat.id)
					.Key("route_length"s).Value((*info)->distance)
					.Key("stop_count"s).Value((*info)->number_stops)
					.Key("unique_stop_count"s).Value((*info)->unique_stops)
				    .EndDict();
			}
        } else if (stat.type == "Map")
        {
			builder.StartDict()
				.Key("request_id"s).Value(stat.id)
				.Key("map"s).Value(answer_map)
				.EndDict();
        }
    }
	builder.EndArray();
    document_answer = builder.Build();
}

void JsonReader::ErrorMassage(json::Builder& builder, int id)
{
	builder.StartDict()
		.Key("request_id"s).Value(id)
		.Key("error_message"s).Value("not found"s)
		.EndDict();
}

void JsonReader::ParseBase(const json::Node& node_)
{
    auto& nodes = node_.AsArray();
    for (auto& node : nodes) {
        auto& tag = node.AsMap();
        if (tag.at("type"s).AsString() == "Stop"s)
        {
            const auto name_ = tag.at("name"s).AsString();
            double lat = tag.at("latitude"s).AsDouble();
            double lng = tag.at("longitude"s).AsDouble();
            stops_.push_back({ name_, lat, lng });

            //дистанция
            if (tag.count("road_distances"s)) {
                auto& distances = tag.at("road_distances"s).AsMap();
                for (const auto&[name, value] : distances) {
                    distances_[{name_, name}] = value.AsInt();
                }
            }
        }
        else if (tag.at("type"s).AsString() == "Bus"s)
        {
            const auto name = tag.at("name"s).AsString();
            const auto round = tag.at("is_roundtrip"s).AsBool();
            auto& it = tag.at("stops"s).AsArray();

            std::vector<std::string> stops_;
            stops_.reserve(it.size());

            for (const auto &stop : it) {
                if (stop.IsString()) {
                    stops_.push_back(stop.AsString());
                }
            }
            buses_.push_back({ name,stops_,round });
        }
        else {
            throw std::invalid_argument("Unknown type"s);
        }
    }
}

void JsonReader::ParseStats(const json::Node& node_)
{
    auto& nodes = node_.AsArray();
    for (auto& node : nodes) {
        const auto& tag = node.AsMap();
        const auto& type = tag.at("type"s).AsString();
        //{id,type,name}
        if (type == "Stop"s || type == "Bus"s)
        {
            stats_.push_back({ tag.at("id"s).AsInt(),type,tag.at("name"s).AsString() });
        }
        else if (type == "Map"s)
        {
            //{ "id": 1, "type": "Map" },
            stats_.push_back({ tag.at("id"s).AsInt(), type, type });
        }
        else {
            throw std::invalid_argument("Unknown type"s);
        }
    }
}

void JsonReader::ParseSettings(const json::Node& node_)
{
    auto& settings = node_.AsMap();

    if (settings.count("width"s)) {
        render_settings.width = settings.at("width"s).AsDouble();
    }
    if (settings.count("height"s)) {
        render_settings.height = settings.at("height"s).AsDouble();
    }
    if (settings.count("padding"s)) {
        render_settings.padding = settings.at("padding"s).AsDouble();
    }
    if (settings.count("line_width"s)) {
        render_settings.line_width = settings.at("line_width"s).AsDouble();
    }
    if (settings.count("stop_radius"s)) {
        render_settings.stop_radius = settings.at("stop_radius"s).AsDouble();
    }
    if (settings.count("bus_label_font_size"s)) {
        render_settings.bus_label_font_size = settings.at("bus_label_font_size"s).AsDouble();
    }
    if (settings.count("bus_label_offset"s)) {
        auto it = settings.at("bus_label_offset"s).AsArray();
        render_settings.bus_label_offset = { it[0].AsDouble(), it[1].AsDouble() };
    }
    if (settings.count("stop_label_font_size"s)) {
        render_settings.stop_label_font_size = settings.at("stop_label_font_size"s).AsDouble();
    }
    if (settings.count("stop_label_offset"s)) {
        auto it = settings.at("stop_label_offset"s).AsArray();
        render_settings.stop_label_offset = { it[0].AsDouble(), it[1].AsDouble() };
    }

    if (settings.count("underlayer_color"s)) {
        GetColor(settings.at("underlayer_color"s), &render_settings.underlayer_color);
    }
    if (settings.count("underlayer_width"s)) {
        render_settings.underlayer_width = settings.at("underlayer_width"s).AsDouble();
    }
    //массив цветов
    if (settings.count("color_palette"s)) {
        auto& array = settings.at("color_palette"s).AsArray();
        render_settings.color_palette.reserve(array.size());
        for (auto& node : array) {
            render_settings.color_palette.push_back({});
            GetColor(node, &render_settings.color_palette.back());
        }
    }
}

void JsonReader::GetColor(const json::Node& node, svg::Color* color) {
    if (node.IsString()) {
        *color = node.AsString();
    }
    else if (node.AsArray().size() == 3) {
        *color = svg::Rgb({ node.AsArray()[0].AsInt(),node.AsArray()[1].AsInt(),node.AsArray()[2].AsInt() });
    }
    else if (node.AsArray().size() == 4) {
        *color = svg::Rgba({ node.AsArray()[0].AsInt(),node.AsArray()[1].AsInt(),node.AsArray()[2].AsInt(), node.AsArray()[3].AsDouble() });
    }
}
