#include "transport_catalogue.h"

#include <utility>
#include <algorithm>

using namespace std::literals;
using namespace std::string_view_literals;
using namespace transport_catalogue;

domain::Stop* TransportCatalogue::FindStop(std::string_view stop_name) const
{
	if (!stops_to_stop_.count(stop_name)) {
		return nullptr;
	}
	return stops_to_stop_.at(stop_name);
}

domain::Bus* TransportCatalogue::FindBus(std::string_view route_name) const
{
	if (!buses_to_bus_.count(route_name)) {
		return nullptr;
	}
	return buses_to_bus_.at(route_name);
}

std::optional<const domain::Bus*> TransportCatalogue::GetBusInfo(std::string_view name) const {

	const auto bus = FindBus(name);
	if (!bus) {
		return std::nullopt;
	}
	return bus;
}

std::optional<const domain::Stop*> TransportCatalogue::GetStopInfo(std::string_view name) const {

	const auto stop = FindStop(name);
	if (!stop) {
		return std::nullopt;
	}
	return stop;
}

void TransportCatalogue::AddStop(std::string_view stop_name, geo::Coordinates coordinate) noexcept
{
    domain::Stop stop{static_cast<std::string>( stop_name), coordinate };
	//добавление в контейнеры
	stops_.push_back(std::move(stop));
	stops_to_stop_.insert({ stops_.back().name, &stops_.back() });
}

void TransportCatalogue::AddBus(std::string_view route_name,
                                const std::vector<std::string>& stops, const bool& ring)
{
	auto it = std::lower_bound(sorted_buses_.begin(), sorted_buses_.end(), route_name);
	if (it != sorted_buses_.end() && *it == route_name) {
		return;
	}

	domain::Bus bus;
	//добавление имени
	bus.name = route_name;
	buses_.push_back(std::move(bus));
	//add отсортированный ветор 
	sorted_buses_.emplace(it, buses_.back().name);

	std::vector<const domain::Stop*> tmp_stops(stops.size());

	//из названий в указатели на существующие остановки
	std::transform(stops.begin(), stops.end(), tmp_stops.begin(), [&](std::string_view element) {
		return stops_to_stop_[element]; });

	//если остановок нет
	if (tmp_stops.empty()) {
		buses_to_bus_.insert({ buses_.back().name, &(buses_.back()) });
		return;
	}
	//запись указателей на уникальные остановки для подсчета
	std::unordered_set<const domain::Stop*> tmp_unique_stops;
	std::for_each(tmp_stops.begin(), tmp_stops.end(), [&](const domain::Stop* stop) { tmp_unique_stops.insert(stop);});
	//добавление уникаольных остановок в деку
	buses_.back().unique_stops = static_cast<int>(tmp_unique_stops.size());

	//если линейный, то добавление обратного направления
	if (!ring) {
		//размер * 2 -1 ( A-B-C-B-A)
		tmp_stops.reserve(tmp_stops.size() * 2 - 1);

		for (auto it = tmp_stops.end() - 2; it != tmp_stops.begin(); --it) {
			tmp_stops.push_back(*it);
		}
		tmp_stops.push_back(tmp_stops.front());
	}
	else {
		buses_.back().route_type = true;
	}
	//add stops
	buses_.back().stops = std::move(tmp_stops);
	//количество остановок
	buses_.back().number_stops = static_cast<int>(buses_.back().stops.size());
	buses_to_bus_.insert({ buses_.back().name, &(buses_.back()) });

	//расстояние
	buses_.back().distance = CalculateAllDistance(route_name);
	//извилистость, то есть отношение фактической длины маршрута к географическому рассто¤нию
	buses_.back().curvature = buses_.back().distance / CalculateCurvature(route_name);

	//добавляем автобусы на каждой остановке 
	for (const auto& stop : buses_.back().stops) {
		buses_on_stops_[stop->name].insert(buses_.back().name);
	}
}

std::optional<std::set<std::string_view>> TransportCatalogue::GetBusesOnStop(std::string_view name) const
{
	//нет остановки
	const auto stop = FindStop(name);
	if (!stop) {
		return std::nullopt;
	}
	std::set<std::string_view > results_null = {};
	//возвращает пустой set
	if (buses_on_stops_.count(stop->name) == 0)
		return results_null;
	//есть остновка, нет автобуссов проходящих через нее and есть автобусы проходящие через нее
	return buses_on_stops_.at(stop->name);
}

void TransportCatalogue::SetDistance(std::string_view stop_from, std::string_view stop_to, int distance) noexcept
{
	distances_[{ FindStop(stop_from), FindStop(stop_to) }] = distance;
}

int TransportCatalogue::GetDistance(std::string_view stop_from,std::string_view stop_to) const
{
	const auto lhs = FindStop(stop_from);
	const auto rhs = FindStop(stop_to);

	if (distances_.count({ lhs, rhs })) {
		return distances_.at({ lhs, rhs });
	}

	if (distances_.count({ rhs, lhs })) {
		return distances_.at({ rhs, lhs });
	}
	return static_cast<int>(geo::ComputeDistance(lhs->coordinate, rhs->coordinate));
}

int TransportCatalogue::CalculateAllDistance(std::string_view route_name) const
{
	auto bus = FindBus(route_name);
	auto stops = bus->stops;
	int result = 0;
	if (!stops.empty()) {
		for (auto iter1 = stops.begin(), iter2 = iter1 + 1; iter2 < stops.end(); ++iter1, ++iter2) {
			result += GetDistance((*iter1)->name, (*iter2)->name);
		}
	}
	return result;
} 

double TransportCatalogue::CalculateCurvature(std::string_view name) const
{
	auto bus = FindBus(name);
	auto stops = bus->stops;
	double results = 0.;
	if (!stops.empty()) {
		for (auto it1 = stops.begin(), it2 = it1 + 1; it2 < stops.end(); ++it1, ++it2) {
			results += geo::ComputeDistance((*it1)->coordinate, (*it2)->coordinate);
		}
	}
	return results;
}
