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

//#include "graph.h"
//#include "router.h"

#include "transport_router.h"


int main() {
#ifdef _DEBUG
	//if (freopen("input.txt.txt", "r", stdin) == nullptr) {
	if (freopen("input_2.txt.txt", "r", stdin) == nullptr) {
	//if (freopen("input_22.txt.txt", "r", stdin) == nullptr) {
	//if (freopen("input_3.txt.txt", "r", stdin) == nullptr) {
	//if (freopen("input_4.txt.txt", "r", stdin) == nullptr) {
		puts("can't open input.txt.txt");
		return 1;
	}
#endif 

	transport_catalogue::TransportCatalogue tc;
	transport_catalogue::InputReaderJson reader(std::cin);
	(void)reader.ReadInputJsonRequest();

	reader.UpdStop(tc);
	reader.UpdBus(tc);
	reader.UpdStopDist(tc);
	reader.UpdRouteSettings(tc);
	RenderData rd = reader.GetRenderData();
	MapRenderer mapdrawer(rd);

	//создаю объекты классов граф и роутер и заполняю данными 
	/*std::set<domain::Stop*, transport_catalogue::StopPointerComparer> stop_set = tc.GetStopSet(); // получаю количество остановок
	graph::DirectedWeightedGraph<double> graph(stop_set.size()*2); // создаю граф с 2 вершинами
	graph::ActivityProcessor activityprocessor(graph, tc);*/


	////////////******************




	////////////////******************

	//graph::ActivityProcessor activityprocessor(tc);

	//graph::Router<double> router = activityprocessor.ProcessRoute();


	//reader.ManageOutputRequests(tc, mapdrawer, activityprocessor, router);

	
	graph::ActivityProcessor activityprocessor(tc);

	reader.ManageOutputRequests(tc, mapdrawer, activityprocessor);
    //activityprocessor.FreeMemory();

	//reader.ManageOutputRequests(tc, mapdrawer);


	return 0;


}