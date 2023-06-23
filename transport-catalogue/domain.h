#pragma once
#include "geo.h"

#include <string>
#include <vector>
#include "deque"

namespace domain {

	struct StopDistancesDescription {
		std::string stop_name;
		std::vector<std::pair<std::string, int>> distances;
	};

	struct Stop {
		std::string stop_name;
		geo::Coordinates coordinates;

	};

	struct BusDescription {
		std::string bus_name;
		std::vector<std::string> stops;
		std::string type;
	};

	struct AllBusInfoBusResponse {
		std::string bus_name;
		int quant_stops;
		int quant_uniq_stops;
		double route_length = 0;
		double route_curvature = 0;
	};



	struct Bus {
		std::string bus_name;
		//std::deque<std::string*> stops;
		std::deque<std::string_view> stops;
		std::string type;
	};

	struct RouteSettings {
		double bus_velocity;
		double bus_wait_time;

	};

}