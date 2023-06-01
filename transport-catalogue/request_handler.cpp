#include "request_handler.h"
#include <sstream>

using namespace std;
void RequestHandler::ReadInputDocument()
{
    reader_.LoadDocument(input_);
    reader_.ReadDocument();
}

void RequestHandler::PrintAnswers()
{
    //считали настройки и передали их в класс MapRenderer
    auto render_settings = reader_.GetRenderSettings();
    map_renderer_.SetRenderSettings(std::move(render_settings));
    //получили обратно сырой текст
    auto answer_map = RenderMap();

    json::Document document_answer;
    reader_.PrintDocument(document_answer,builder,answer_map);
    json::Print(document_answer, output_);
}

void RequestHandler::RenderMapGlob()
{
    map_renderer_.Render(output_);
}

std::string RequestHandler::RenderMap()
{
    std::ostringstream stream;
    map_renderer_.Render(stream);
    return stream.str();
}

void RequestHandler::AddStops()
{
    auto stops_ = reader_.GetStops();
    std::for_each(stops_.begin(), stops_.end(), [&](domain::StopInput& stop)
    { db_.AddStop(stop.name, stop.coordinates);});
}

void RequestHandler::AddBuses()
{
    auto buses_ = reader_.GetBuses();
    std::for_each(buses_.begin(), buses_.end(), [&](domain::BusInput& bus)
    { db_.AddBus(bus.name, bus.stops, bus.is_roundtrip); });
}

void RequestHandler::AddDistances()
{
    auto distances_ = reader_.GetDistances();
    for (auto& dis : distances_)
        db_.SetDistance(dis.first.first, dis.first.second, dis.second);
}

std::optional<const domain::Bus*> RequestHandler::GetBusStat(const std::string_view &bus_name) const
{
    return db_.GetBusInfo(std::string(bus_name));
}

std::optional<std::set<std::string_view>> RequestHandler::GetBusesByStop(const std::string_view &stop_name) const
{
    return db_.GetBusesOnStop(std::string(stop_name));
}
