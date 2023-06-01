#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <chrono>

#include "request_handler.h"

using namespace std;
using namespace std::literals;

void read_file()
{
    std::ifstream in("s10_final_opentest_1.json");
    if (in.is_open()) {
        //файл вывода
        std::filebuf file;
        file.open("s10_final_opentest_1_answer_.json", std::ios::out);
        std::ostream out(&file);

        transport_catalogue::TransportCatalogue transport_;
        RequestHandler request_handler(transport_, in,out);
        request_handler.ReadInputDocument();
        request_handler.AddStops();
        request_handler.AddDistances();
        request_handler.AddBuses();
        //request_handler.RenderMapGlob();
        request_handler.PrintAnswers();
        file.close();
    }
    in.close();
}

int main()
{
    setlocale(LC_ALL, "Russian");
	//read_file();

    transport_catalogue::TransportCatalogue transport_;
    RequestHandler request_handler(transport_,std::cin, std::cout);
    request_handler.ReadInputDocument();
    request_handler.AddStops();
    request_handler.AddDistances();
    request_handler.AddBuses();
    //request_handler.RenderMapGlob();
    request_handler.PrintAnswers();

    return 0;
}