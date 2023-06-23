#pragma once

#include "domain.h"

#include <string>
#include <unordered_set>
#include "geo.h"
#include <functional> 
#include "set"
#include "deque"

namespace transport_catalogue {
	struct OutputRequest {
		int id;
		std::string type;
		std::string name;

		std::string from;
		std::string to;




	};

	struct StopComparer {
		bool operator()(const domain::Stop& lhs, const domain::Stop& rhs) const {
			return lhs.stop_name < rhs.stop_name;
		}
	};

	struct StopPointerComparer {
		bool operator()(const domain::Stop* lhs, const domain::Stop* rhs) const {
			return lhs->stop_name < rhs->stop_name;
		}
	};


	namespace detail {
		struct PairOfStopPointerUsingString {
			std::size_t operator()(const std::pair<const  domain::Stop*, const  domain::Stop*>& p) const {
				const std::size_t constantal = 31;
				std::size_t hash_value_one = 0;
				std::size_t hash_value_two = 0;
				std::size_t p_pow = 1;
				for (char c : p.first->stop_name) {
					hash_value_one = (hash_value_one + (c - 'a' + 1) * p_pow) % std::size_t(-1);
					p_pow = (p_pow * constantal) % std::size_t(-1);
				}

				for (char c : p.second->stop_name) {
					hash_value_one = (hash_value_two + (c - 'a' + 1) * p_pow) % std::size_t(-1);
					p_pow = (p_pow * constantal) % std::size_t(-1);
				}

				return hash_value_one + hash_value_two * constantal;
			}
		};
	}


	class TransportCatalogue {
	public:
		void AddBus(const domain::BusDescription& bus);
		void AddStop(domain::Stop stop);
		const domain::Bus* FindBus(std::string_view bus) const;
		virtual const domain::Stop* FindStop(std::string_view stop) const;
		domain::AllBusInfoBusResponse GetAllBusInfo(std::string_view bus) const;
		std::set<std::string> GetStopInfo(std::string_view s) const;
		void AddStopDistance(domain::StopDistancesDescription distance);
		int GetStopDistance(const domain::Stop& s1, const domain::Stop& s2)  const;

		const std::deque<domain::Bus>& GetBuses() const;
		const std::deque<domain::Stop>& GetStops() const ;

		// добавлено на 13 спринт
		void AddRouteSettings(const domain::RouteSettings route_settings);
		double GetWaitTime();
		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, double, detail::PairOfStopPointerUsingString> GetstopsDistanceTime();
		


		double GetVelocity();
		size_t GetStopsQuantity();
		


	private:
		double bus_wait_time_ = 6;  // добавлено на 13 спринт
		double bus_velocity_ = 40; // добавлено на 13 спринт
		std::deque<domain::Bus> buses_;
		std::deque<domain::Stop> stops_;
		
		std::unordered_map<std::string_view, domain::Stop*> stop_name_to_stop_;
		std::unordered_map<std::string_view, domain::Bus*> bus_name_to_bus_;
		std::unordered_map<std::string_view, std::set<std::string>> stop_info_;
		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, detail::PairOfStopPointerUsingString> stops_distance_;
		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, double, detail::PairOfStopPointerUsingString> stops_distance_time_;

		// добавоено на 13 спринт 

	};
}