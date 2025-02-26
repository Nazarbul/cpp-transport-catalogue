#pragma once

#include "map_render.h"
#include "transport_catalogue.h"

using namespace transport_catalogue;
using namespace map_render;

namespace request_handler
{
    class RequestHandler
    {
    public:
        RequestHandler(const TransportCatalogue &catalogue,
                       const MapRender &render_settings) : catalogue_(catalogue),
                                                           render_settings_(render_settings) {};
        svg::Document RenderMap() const;
        bool SearchStopName(const std::string_view stopname) const;
        bool SearchBusNumber(const std::string_view busname) const;
        const std::set<std::string>& GetBusesOnStop(std::string_view stopname) const;
        std::optional<domain::BusInfo> GetBusInfo(const std::string_view busname) const;

    private:
        const TransportCatalogue &catalogue_;
        const MapRender &render_settings_;
    };
}