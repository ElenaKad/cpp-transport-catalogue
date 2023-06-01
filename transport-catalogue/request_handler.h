#pragma once

#include "json_reader.h"

#include <algorithm>

/*
* Класс RequestHandler играет роль Фасада,
* упрощающего взаимодействие JSON reader-а
* с другими подсистемами приложения.
* См. паттерн проектирования Фасад:
* https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
*/

class RequestHandler {
public:
   explicit RequestHandler(transport_catalogue::TransportCatalogue &db,
                   std::istream &input, std::ostream &output)
        : input_(input),output_(output), db_(db),reader_(db_),map_renderer_(db_){}

    void ReadInputDocument();
    //печать ответа
    void PrintAnswers();

    //функции добавления информации в транспортный справочник
    void AddStops();
    void AddBuses();
    void AddDistances();

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<const domain::Bus *> GetBusStat(const std::string_view &bus_name) const;

    // Возвращает маршруты, проходящие через
    std::optional<std::set<std::string_view>> GetBusesByStop(const std::string_view &stop_name) const;

	// Этот метод будет нужен в следующей части итогового проекта
	void RenderMapGlob();

private:
	std::istream &input_ = std::cin;
	std::ostream &output_ = std::cout;

	std::string RenderMap();
    //RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    transport_catalogue::TransportCatalogue &db_;
    json_reader::JsonReader reader_;
    renderer::MapRenderer map_renderer_;
	json::Builder builder;
};
