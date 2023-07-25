#pragma once

#include "transport_catalogue.h"
#include "svg.h"
#include "map_renderer.h"

namespace transport_catalogue {

    class RequestHandler {
    public:
        
        RequestHandler(transport_catalogue::TransportCatalogue& tc, MapRenderer& renderer);
                
        void RenderMapByString();

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        transport_catalogue::TransportCatalogue& tc_;
        MapRenderer& renderer_;
    };
}