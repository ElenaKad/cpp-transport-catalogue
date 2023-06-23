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
        
        transport_catalogue::TransportCatalogue& tc_;
        MapRenderer& renderer_;
    };
}