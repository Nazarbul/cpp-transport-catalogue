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

const std::set<std::string> &RequestHandler::GetBusesOnStop(const std::string_view stopname) const
{
    return catalogue_.FindStop(stopname)->buses;
}

bool RequestHandler::SearchBusNumber(const std::string_view busname) const
{
    return catalogue_.FindBus(busname);
}

std::optional<domain::BusInfo> RequestHandler::GetBusInfo(const std::string_view busname) const
{
    const auto &bus_info = catalogue_.BusInfo(busname);
    return bus_info;
}
