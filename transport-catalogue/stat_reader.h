#pragma once

#include "transport_catalogue.h"
#include <utility>

namespace transport {

void ProcessRequests(std::istream& in,Catalogue & catalogue);

namespace detail {

void PrintRoute(std::ostream& out, std::string& line, Catalogue& catalogue);
void PrintStop(std::ostream& out, std::string& line, Catalogue& catalogue);


		
		

}// namespace detail
}// namespace transport
