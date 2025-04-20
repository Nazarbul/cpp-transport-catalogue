#pragma once

#include "map_render.h"
#include "transport_catalogue.h"
#include "transport_router.h"

using namespace transport_catalogue;
using namespace map_render;

namespace request_handler
{
    class RequestHandler
    {
    public:
        RequestHandler(const TransportCatalogue &catalogue,
                       const MapRender &render_settings, const route::Router &router) : catalogue_(catalogue),
                                                                                        render_settings_(render_settings), router_(router) {};
        svg::Document RenderMap() const;
        bool SearchStopName(const std::string_view stopname) const;
        bool SearchBusNumber(const std::string_view busname) const;
        const std::set<std::string> &GetBusesOnStop(std::string_view stopname) const;
        std::optional<domain::BusInfo> GetBusInfo(const std::string_view busname) const;
        std::optional<std::pair<graph::Router<route::RouteWeight>::RouteInfo, std::vector<route::RouteWeight>>> GetRouting(std::string_view from,
                                                                                                                           std::string_view to) const;

    private:
        const TransportCatalogue &catalogue_;
        const MapRender &render_settings_;
        const route::Router &router_;
    };
}