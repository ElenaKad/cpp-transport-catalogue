#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "svg.h"
#include "json_builder.h"

#include <iostream>

namespace json_reader {

// класс реализующий парсинг входного потока данных
// транспортного каталога в формате JSON
class JsonReader {
public:
    //считывает все данные из входного потока
    explicit  JsonReader(transport_catalogue::TransportCatalogue &catalogue)
        : catalogue_(catalogue){}

    const std::vector<domain::BusInput> GetBuses()const;
    const std::vector<domain::StopInput> GetStops()const;
    const std::map<std::pair<std::string, std::string>, int> GetDistances()const;
    const renderer::RenderSettings GetRenderSettings()const {return render_settings;}

    void LoadDocument(std::istream &input);
    void ReadDocument();

    //печать документа в ноду
    void PrintDocument(json::Document& document_answer, json::Builder& builder, const std::string& answer_map);

private:
    // загрузка данных из json
    void ParseBase(const json::Node& node_);
    void ParseStats(const json::Node& node_);
    void ParseSettings(const json::Node& node_);
    //получение различных форматов цвета
    void GetColor(const json::Node& node, svg::Color* color);
	//печать строки ошибки
	void ErrorMassage(json::Builder& builder, int id);

    //входные параметры
    std::vector<domain::BusInput> buses_; // маршруты(автобусы)
    std::vector<domain::StopInput> stops_; // остановки
    std::vector<domain::query> stats_; // запрос базы
    std::map<std::pair<std::string, std::string>, int> distances_;// расстояние от остановки до остановки

    json::Document document_ = {};

    renderer::RenderSettings render_settings;
    transport_catalogue::TransportCatalogue &catalogue_;
};
}
