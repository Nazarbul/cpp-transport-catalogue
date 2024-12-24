#include "stat_reader.h"

#include <iomanip>
#include <sstream>

using namespace std::literals;

namespace transport_catalogue::output::detail {
    void PrintBus(const std::set<Bus*>& buses, std::ostream& output) {
        for (const auto bus : buses) {
            output << ' ' << bus->name_;
        }
    }
    void PrintBusInfo(const BusInfo& bus, std::ostream& output){
        if (bus.name_.empty()) {
            output<<": not found"s << std::endl;
        }
        else {
            output <<bus.name_ << ": "s 
            << bus.count_all_stops << " stops on route, "s 
            << bus.count_unique_stops
            << " unique stops, "s << std::setprecision(6) 
            << bus.route_length 
            << " route length"s << std::endl;
        }
    }
}
void transport_catalogue::output::ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
    std::ostream& output) {

    size_t symbol = request.find(' ');
    size_t symbol1 = request.find_first_not_of(' ', symbol);
    std::string_view command = request.substr(0, symbol);
    request = request.substr(symbol1);
    if (command == "Bus"s) {
        output<<command<<' '<<request;
        BusInfo bus = transport_catalogue.GetBusInfo(request);
        detail::PrintBusInfo(bus, output);
    }
    if (command == "Stop"s) {
        const Stop* stop = transport_catalogue.FindStop(request);
        if (stop == nullptr) {
            output << command << ' ' << request << ": not found"s << std::endl;
        }
        else {
            const std::set<Bus*>& result = transport_catalogue.GetStopInfo(stop->name_);
            if (result.empty()) {
                output << command << ' ' << stop->name_ << ": no buses"s << std::endl;
            }
            else {
                output << command << ' ' << stop->name_ << ": buses"s;
                detail::PrintBus(result, output);
                output << std::endl;
            }
        }
    }
}

void transport_catalogue::output::ReadForStreamandWriteToStream(const TransportCatalogue& transport_catalogue, 
            std::istream& input, std::ostream& output){
    int stat_request_count;
    input >> stat_request_count >> std::ws;
    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        getline(input, line);
        transport_catalogue::output::ParseAndPrintStat(transport_catalogue, line, output);
    }
}