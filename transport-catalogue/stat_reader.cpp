#include "stat_reader.h"

#include <iomanip>
#include <sstream>

using namespace std::literals;

namespace transport_catalogue::output::detail {
    void PrintBus(std::set<std::string_view> buses, std::ostream& output) {
        for (const auto bus : buses) {
            output << ' ' << bus;
        }
    }
}
void transport_catalogue::output::ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
    std::ostream& output) {

    size_t symbol = request.find(' ');
    size_t symbol1 = request.find_first_not_of(' ', symbol);
    std::string_view command = request.substr(0, symbol);
    request = request.substr(symbol1);
    if (command == "Bus"s) {
        Bus* bus = tansport_catalogue.FindBus(request);

        if (bus == nullptr) {
            output << command << ' ' << request << ": not found"s << std::endl;
        }
        else {
            output << command << ' ' << bus->name_ << ": "s 
            << bus->stops_.size() << " stops on route, "s 
            << tansport_catalogue.GetUniqueStops(bus) 
            << " unique stops, "s << std::setprecision(6) 
            << tansport_catalogue.GetLengthRoute(bus) 
            << " route length"s << std::endl;
        }
    }
    if (command == "Stop"s) {
        Stop* stop = tansport_catalogue.FindStop(request);
        if (stop == nullptr) {
            output << command << ' ' << request << ": not found"s << std::endl;
        }
        else {
            std::set<std::string_view> result = tansport_catalogue.GetBusInfo(stop->name_);
            if (result.empty()) {
                output << command << ' ' << stop->name_ << ": no buses"s << std::endl;
            }
            else {
                output << command << ' ' << stop->name_ << ": buses"s;
                output::detail::PrintBus(result, output);
                output << std::endl;
            }
        }
    }
}