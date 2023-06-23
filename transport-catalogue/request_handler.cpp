#include "request_handler.h"
#include <sstream>
#include <iostream>
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
namespace transport_catalogue {

    RequestHandler::RequestHandler(TransportCatalogue& tc,  /*renderer::*/ MapRenderer& renderer) : tc_(tc), renderer_(renderer) {}

    // Возвращает информацию о маршруте (запрос Bus)
    //std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    //const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    /*void RequestHandler::RenderMap() {
        renderer_.DrawRoute(tc_);
    };
    */

    void RequestHandler::RenderMapByString() {
        std::string str = renderer_.DrawRouteGetDoc(tc_);

        std::stringstream ss;
        ss << str;
        std::string result = ss.str();
        std::cout << result << std::endl;





    }

}