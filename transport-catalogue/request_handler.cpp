#include "request_handler.h"
#include <sstream>
#include <iostream>

namespace transport_catalogue {

    RequestHandler::RequestHandler(TransportCatalogue& tc, MapRenderer& renderer) : tc_(tc), renderer_(renderer) {}

    void RequestHandler::RenderMapByString() {
        std::string str = renderer_.DrawRouteGetDoc(tc_);

        std::stringstream ss;
        ss << str;
        std::string result = ss.str();
        std::cout << result << std::endl;
    }

}