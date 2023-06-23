#include "transport_catalogue.h"
#include "json.h"
#include "geo.h"
#include "svg.h"
#include "json_reader.h"
#include "json_builder.h"

//#include "graph.h"
//include "router.h"
//#include "transport_router.h"
//#include "transport_router.h"


using namespace json;
using namespace std;
using namespace domain;
using namespace svg;

namespace transport_catalogue {

	svg::Color GetColor(const json::Node& el) {
		svg::Color color;

		if (el.IsString()) {
			color = el.AsString();
		}
		else {
			if (el.AsArray().size() == 3) {
				Rgb rgb;
				rgb.red = static_cast<uint8_t>(el.AsArray()[0].AsInt());
				rgb.green = static_cast<uint8_t>(el.AsArray()[1].AsInt());
				rgb.blue = static_cast<uint8_t>(el.AsArray()[2].AsInt());
				color = rgb;
			}
			else if (el.AsArray().size() == 4) {
				Rgba rgba;
				rgba.red = static_cast<uint8_t>(el.AsArray()[0].AsInt());
				rgba.green = static_cast<uint8_t>(el.AsArray()[1].AsInt());
				rgba.blue = static_cast<uint8_t>(el.AsArray()[2].AsInt());
				rgba.opacity = el.AsArray()[3].AsDouble();
				color = rgba;
			}
		}


		return color;

	}


	InputReaderJson::InputReaderJson(istream& is) : is_(is), load_(json::Load(is)) {

	}


	void InputReaderJson::ReadInputJsonBaseRequest() {
		const auto& json_array = ((load_.GetRoot()).AsDict()).at("base_requests"s);
		for (const auto& file : json_array.AsArray()) {
			const auto& json_obj = file.AsDict();
			if (json_obj.at("type"s) == "Stop"s) {
				Stop stopjson;
				stopjson.stop_name = json_obj.at("name").AsString();
				stopjson.coordinates.lat = json_obj.at("latitude").AsDouble();
				stopjson.coordinates.lng = json_obj.at("longitude").AsDouble();

				StopDistancesDescription input_stop_dist;
				input_stop_dist.stop_name = json_obj.at("name").AsString();
				auto heighbors = json_obj.at("road_distances");

				for (auto el : heighbors.AsDict()) {
					input_stop_dist.distances.push_back(make_pair(el.first, el.second.AsInt()));

				}
				upd_req_stop_.push_back(stopjson);
				distances_.push_back(input_stop_dist);
			}
			else if (json_obj.at("type"s) == "Bus"s) {
				BusDescription bs;
				auto stop_list = json_obj.at("stops").AsArray();
				for (auto el : stop_list) {

					bs.stops.push_back(el.AsString());

				}
				bs.bus_name = json_obj.at("name").AsString();

				auto trip = json_obj.at("is_roundtrip").AsBool();
				if (trip) {
					bs.type = "true"s;
				}
				else { bs.type = "false"s; }

				upd_req_bus_.push_back(bs);

			}
		}

	}
    
    void InputReaderJson::ReadInputJsonStatRequest() {
		const auto& json_array_out = ((load_.GetRoot()).AsDict()).at("stat_requests"s);
		if (!json_array_out.IsNull()) {
			for (const auto& file : json_array_out.AsArray()) {
				const auto& json_obj = file.AsDict();
				OutputRequest outputstopjson;
				if (json_obj.at("type").AsString() == "Map"s) {
					outputstopjson.id = json_obj.at("id").AsInt();
					outputstopjson.type = json_obj.at("type").AsString();
					out_req_.push_back(outputstopjson);

				} else if (json_obj.at("type").AsString() == "Route"s) {
					outputstopjson.id = json_obj.at("id").AsInt();
					outputstopjson.type = json_obj.at("type").AsString();
					outputstopjson.from = json_obj.at("from").AsString();
					outputstopjson.to = json_obj.at("to").AsString();

					out_req_.push_back(outputstopjson);
				} else {
					outputstopjson.name = json_obj.at("name").AsString();
					outputstopjson.id = json_obj.at("id").AsInt();
					outputstopjson.type = json_obj.at("type").AsString();
					out_req_.push_back(outputstopjson);
				}
			}
		}

	}


	void InputReaderJson::ReadInputJsonRenderSettings() {

		const auto& json_array_render = ((load_.GetRoot()).AsDict()).at("render_settings"s).AsDict();
		if (json_array_render.find("width") != json_array_render.end()) {
			render_data_.width = json_array_render.find("width")->second.AsDouble();
		}

		if (json_array_render.find("height") != json_array_render.end()) {
			render_data_.height = json_array_render.find("height")->second.AsDouble();
		}

		if (json_array_render.find("padding") != json_array_render.end()) {
			render_data_.padding = json_array_render.find("padding")->second.AsDouble();
		}

		if (json_array_render.find("line_width") != json_array_render.end()) {
			render_data_.line_width = json_array_render.find("line_width")->second.AsDouble();
		}

		if (json_array_render.find("stop_radius") != json_array_render.end()) {
			render_data_.stop_radius = json_array_render.find("stop_radius")->second.AsDouble();
		}

		if (json_array_render.find("bus_label_font_size") != json_array_render.end()) {
			render_data_.bus_label_font_size = json_array_render.find("bus_label_font_size")->second.AsDouble();
		}

		if (json_array_render.find("bus_label_offset") != json_array_render.end()) {

			for (auto el : json_array_render.find("bus_label_offset")->second.AsArray()) { render_data_.bus_label_offset.push_back(el.AsDouble()); }
		}

		if (json_array_render.find("stop_label_offset") != json_array_render.end()) {

			for (auto el : json_array_render.find("stop_label_offset")->second.AsArray()) { render_data_.stop_label_offset.push_back(el.AsDouble()); }
		}


		if (json_array_render.find("stop_label_font_size") != json_array_render.end()) {
			render_data_.stop_label_font_size = json_array_render.find("stop_label_font_size")->second.AsDouble();
		}

		if (json_array_render.find("underlayer_color") != json_array_render.end()) {

			render_data_.underlayer_color.push_back(GetColor(json_array_render.find("underlayer_color")->second));
		}

		if (json_array_render.find("underlayer_width") != json_array_render.end()) {
			render_data_.underlayer_width = json_array_render.find("underlayer_width")->second.AsDouble();
		}

		if (json_array_render.find("color_palette") != json_array_render.end()) {
			for (auto el : json_array_render.find("color_palette")->second.AsArray()) {

				render_data_.color_palette.push_back(GetColor(el));

			}

		}

	}



	void InputReaderJson::ReadInputJsonRouteSettings() {
		const auto& json_array_out = ((load_.GetRoot()).AsDict()).at("routing_settings"s);
		const auto& json_obj = json_array_out.AsDict();
		route_settings_.bus_velocity = json_obj.at("bus_velocity").AsDouble();
		route_settings_.bus_wait_time = json_obj.at("bus_wait_time").AsDouble();
	}

	void InputReaderJson::ReadInputJsonRequest() {
		ReadInputJsonBaseRequest();
		ReadInputJsonStatRequest();
		ReadInputJsonRenderSettings();
		ReadInputJsonRouteSettings();
	}


	void InputReaderJson::UpdStop(TransportCatalogue& tc) {
		for (int i = 0; i < static_cast<int>(upd_req_stop_.size()); ++i) {
			tc.AddStop(upd_req_stop_[i]);
		}
	}

	void InputReaderJson::UpdStopDist(TransportCatalogue& tc) {
		for (int i = 0; i < static_cast<int>(distances_.size()); ++i) {
			tc.AddStopDistance(distances_[i]);
		}
	}

	void InputReaderJson::UpdBus(TransportCatalogue& tc) {
		for (int i = 0; i < static_cast<int>(upd_req_bus_.size()); ++i) {
			tc.AddBus(upd_req_bus_[i]);
		}
	}

	/*graph::ActivityProcessor<Weight>& actprocess )*/



	RenderData InputReaderJson::GetRenderData() {
		return render_data_;
	}


	void InputReaderJson::UpdRouteSettings(TransportCatalogue& tc) {

		tc.AddRouteSettings(route_settings_);

	}

}