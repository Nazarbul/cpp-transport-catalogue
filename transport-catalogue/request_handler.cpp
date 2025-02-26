#include "request_handler.h"

using namespace request_handler;


svg::Document RequestHandler::RenderMap() const
{
    auto buses = catalogue_.GetBuses();
    return render_settings_.RenderMap(buses);
}

bool RequestHandler::SearchStopName(const std::string_view stopname) const
{
    return catalogue_.FindStop(stopname);
}

const std::set<std::string>& RequestHandler::GetBusesOnStop(const std::string_view stopname) const
{
    return catalogue_.FindStop(stopname)->buses;
}

bool RequestHandler::SearchBusNumber(const std::string_view busname) const
{
    return catalogue_.FindBus(busname);
}

std::optional<domain::BusInfo> RequestHandler::GetBusInfo(const std::string_view busname) const
{
    domain::BusInfo bus_info;
    const auto bus = catalogue_.FindBus(busname);
    if (bus)
    {
        bus_info.name_ = busname;
        bus_info.count_unique_stops = catalogue_.GetUniqueStops(busname);
        bus_info.count_all_stops = bus->stops_.size();
        bus_info.route_length = catalogue_.GetDistances(busname);
        bus_info.geo_length = catalogue_.GetLengthRoute(busname);
        bus_info.courvature = bus_info.route_length / bus_info.geo_length;
    }
    return bus_info;
}
