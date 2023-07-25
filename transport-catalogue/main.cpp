#define _CRT_SECURE_NO_WARNINGS
#include "geo.h"
#include "transport_catalogue.h"
#include "json_reader.h"
#include "iostream"
#include "map_renderer.h"
#include "request_handler.h"
#include <fstream>
using namespace transport_catalogue;
using namespace std;
#include <chrono>
#include "serialization.h"
#include "transport_router.h"
#include <string_view>

using namespace std::literals;
void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
#ifdef _DEBUG
        if (freopen("s14_1_opentest_3_make_base.json", "r", stdin) == nullptr) {
                puts("can't open input.txt.txt");
                return 1;
            }
        }
#endif
        transport_catalogue::TransportCatalogue tc;
        transport_catalogue::InputReaderJson reader(std::cin);
        (void)reader.ReadInputJsonRequestForFillBase();

        reader.UpdStop(tc);
        reader.UpdBus(tc);
        reader.UpdStopDist(tc);
        reader.UpdRouteSettings(tc);
        reader.UpdSerializeSettings(tc);

        RenderSettings rd = reader.GetRenderSettings();

        ofstream out_file(tc.GetSerializerFilePath(), ios::binary);

        domain::RouteSettings routeSettings = tc.GetRouteSettings();

        serialization::catalogue_serialization(tc, rd , routeSettings, out_file);

       
    }
    else if (mode == "process_requests"sv) {
#ifdef _DEBUG
        if (freopen("s14_1_opentest_3_process_requests.json", "r", stdin) == nullptr) {
					puts("can't open out.txt.txt");
					return 1;
				}
#endif

        transport_catalogue::InputReaderJson reader(std::cin);
        (void)reader.ReadInputJsonRequestForReadBase();

        ifstream in_file(reader.GetSerializeFilePath(), ios::binary);
        auto catalogue = serialization::catalogue_deserialization(in_file);
        RenderSettings rd = catalogue.render_settings_;
        transport_catalogue::TransportCatalogue tc = catalogue.transport_catalogue_;
        tc.AddRouteSettings(catalogue.routing_settings_);

        MapRenderer mapdrawer(rd);
        graph::ActivityProcessor activityprocessor(tc);
        reader.ManageOutputRequests(tc, mapdrawer, activityprocessor);
    }
    else {
        PrintUsage();
        return 1;
    }
}
