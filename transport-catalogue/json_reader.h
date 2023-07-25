#pragma once

#include <sstream>
#include <string>
#include <deque>
#include <iostream>
#include <vector>

#include "transport_catalogue.h"
#include "domain.h"
#include "json.h"
#include "geo.h"
#include "map_renderer.h"
#include "json_builder.h"


#include "transport_router.h"




using namespace json;
using namespace std;
using namespace domain;
using namespace svg;

inline json::Document LoadJSON(const std::string& s) {
	std::istringstream strm(s);
	return json::Load(strm);
}

inline std::string Print(const json::Node& node) {
	std::ostringstream out;
	Print(json::Document{ node }, out);
	return out.str();
}


namespace transport_catalogue {

	class InputReaderJson {
	public:
		explicit InputReaderJson(std::istream& is);

		void ReadInputJsonBaseRequest();
		void ReadInputJsonStatRequest();
		void ReadInputJsonRenderSettings();

		void ReadInputJsonRouteSettings();
		void ReadInputJsonSerializeSettings();


		void ReadInputJsonRequest();

		// Добавлено на 15 спринт 
		void ReadInputJsonRequestForFillBase();
		void ReadInputJsonRequestForReadBase();

		void UpdStop(TransportCatalogue& tc);

		void UpdBus(TransportCatalogue& tc);

		void UpdStopDist(TransportCatalogue& tc);


		void ManageOutputRequests(TransportCatalogue& tc, MapRenderer& mr, graph::ActivityProcessor& actprocess)
		{
			std::ostream& out = std::cout;
			json::Array queries;
			for (const auto& el : out_req_) {
				if (el.type == "Bus"s) {

					const Bus* bus_resp = tc.FindBus(el.name);
					if (bus_resp == nullptr) {


						json::Node answer_empty_bus = json::Builder{}
							.StartDict().Key("error_message").Value("not found"s)
							.Key("request_id").Value(el.id).EndDict().Build();



						queries.emplace_back(answer_empty_bus);

					}

					else {
						AllBusInfoBusResponse r = tc.GetAllBusInfo(el.name);



						json::Node non_empty_bus = json::Builder{}
							.StartDict()
							.Key("curvature").Value(r.route_curvature)
							.Key("request_id").Value(el.id)
							.Key("route_length").Value(r.route_length)
							.Key("stop_count").Value(r.quant_stops)
							.Key("unique_stop_count").Value(r.quant_uniq_stops)
							.EndDict().Build();


						queries.emplace_back(non_empty_bus);
					}
				}

				if (el.type == "Stop"s) {
					const Stop* myStop = tc.FindStop(el.name);
					if (myStop == nullptr) {

						json::Node answer_empty_stop = json::Builder{}
							.StartDict()
							.Key("error_message").Value("not found"s)
							.Key("request_id").Value(el.id)
							.EndDict().Build();


						queries.emplace_back(answer_empty_stop);

					}
					else {
						set<string> r = tc.GetStopInfo(el.name);
						json::Array routes;
						std::copy(r.begin(), r.end(), std::back_inserter(routes));

						json::Node answer_stop = json::Builder{}
							.StartDict()
							.Key("buses").Value(routes)
							.Key("request_id").Value(el.id)
							.EndDict().Build();



						queries.emplace_back(answer_stop);


					}
				}
				if (el.type == "Map"s) {

					string map_str = mr.DrawRouteGetDoc(tc);


					json::Node answer_empty_map = json::Builder{}
						.StartDict()
						.Key("map").Value(map_str)
						.Key("request_id").Value(el.id)
						.EndDict().Build();

					queries.emplace_back(answer_empty_map);


				}

				else if (el.type == "Route"s) {

					if (tc.FindStop(el.from) && tc.FindStop(el.to)) {

						std::optional<graph::DestinatioInfo> route = actprocess.GetRouteAndBuses(el.from, el.to);
						std::vector<json::Node> array;

						int request_id = el.id;
						double total_time = 0;
						std::string error_message = "not found"s;
	
						if (route.has_value()) {
							total_time = route.value().all_time;
							std::vector<std::variant<graph::BusActivity, graph::WaitingActivity>> final_route = route.value().route;

							for (auto el : final_route) {

								if (std::holds_alternative<graph::BusActivity>(el)) {
									graph::BusActivity act = std::get<graph::BusActivity>(el);

									json::Node bus_route_description = json::Builder{}
										.StartDict()
										.Key("bus").Value(act.bus_name)
										.Key("span_count").Value(act.span_count)
										.Key("time").Value(act.time)
										.Key("type").Value("Bus")
										.EndDict().Build();

									array.push_back(bus_route_description);
								}

								else {
									graph::WaitingActivity act = std::get<graph::WaitingActivity>(el);

									json::Node bus_route_description = json::Builder{}
										.StartDict()
										.Key("stop_name").Value(act.stop_name_from)
										.Key("time").Value(act.time)
										.Key("type").Value("Wait")
										.EndDict().Build();

									array.push_back(bus_route_description);

								}

							}
							json::Node final_route_description = json::Builder{}
								.StartDict()
								.Key("items").Value(array)
								.Key("request_id").Value(request_id)
								.Key("total_time").Value(total_time)
								.EndDict().Build();

							queries.emplace_back(final_route_description);

						}
						else {
							int request_id = el.id;
							std::string error_message = "not found"s;
							json::Node final_route_description = json::Builder{}
								.StartDict()
								.Key("request_id").Value(request_id)
								.Key("error_message").Value(error_message)
								.EndDict().Build();
							queries.emplace_back(final_route_description);

						}
					}
					else {
						int request_id = el.id;
						std::string error_message = "not found"s;
						json::Node final_route_description = json::Builder{}
							.StartDict()
							.Key("request_id").Value(request_id)
							.Key("error_message").Value(error_message)
							.EndDict().Build();
						queries.emplace_back(final_route_description);

					}

				}
			}
			json::Print(json::Document{ queries }, out);
		}

        RenderSettings GetRenderSettings();

		void UpdRouteSettings(TransportCatalogue& tc);
		// добавлено на 15 спринт
		void UpdSerializeSettings(TransportCatalogue& tc);

		std::string GetSerializeFilePath();


	private:
		std::istream& is_;

		std::deque<OutputRequest> out_req_;

		std::deque<domain::BusDescription> upd_req_bus_;
		std::deque<domain::Stop> upd_req_stop_;
		std::vector<domain::StopDistancesDescription> distances_;
        RenderSettings render_settings_;
		json::Document load_;
		domain::RouteSettings route_settings_;
		std::string serialize_file_path_;

	};

}


