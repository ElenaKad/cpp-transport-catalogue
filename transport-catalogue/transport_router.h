
#pragma once
#include "router.h"
#include "transport_catalogue.h"

#include <variant>
#include <memory>

namespace graph {



	struct BusActivity {
		std::string bus_name;
		double time;
		int span_count;
	};


	struct WaitingActivity {
		std::string stop_name_from;
		double time;
	};

	struct DestinatioInfo { // такое название потому что RouteInfo уже используется в router 
		std::vector<std::variant<BusActivity, WaitingActivity>> route;
		double all_time=0.0;
	};

	class ActivityProcessor {

	public:
		ActivityProcessor(transport_catalogue::TransportCatalogue& tc);

		void AddKnots();

		std::optional<DestinatioInfo> GetRouteAndBuses(std::string_view stop_name_from, std::string_view stop_name_to);

	private:
		transport_catalogue::TransportCatalogue& tc;
		DirectedWeightedGraph<double> graph_;
		std::unordered_map<std::string_view, size_t> stop_to_vertex_;
		std::unique_ptr<graph::Router<double>> router_;




		std::optional<size_t> GetValueByKey(std::string_view key);

		bool ChekExistValue(std::string_view key);

		void AddStopsOneDirection(const std::deque<std::string_view>& stops, const std::string& bus_name);


		void AddStopsNonRoundTrip(std::deque<std::string_view> stops, const std::string& bus_name);
	};
}