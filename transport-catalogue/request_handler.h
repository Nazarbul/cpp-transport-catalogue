#pragma once

#include "map_render.h"
#include "transport_catalogue.h"
#include "jsonreader.h"

using namespace transport_catalogue;
using namespace transport_catalogue::jsonreader;
using namespace map_render;

namespace request_handler{
    class RequestHandler{
        public:
        RequestHandler() = default;
        std::vector<geo::Coordinates> get_stops_coordinates(TransportCatalogue& catalogue_) const;
        void execute_queries(TransportCatalogue& transport_catalogue, 
        std::vector<StatRequest>& stat_request, Render_settings& render_settings);
        void render_map(MapRender& Map_render, TransportCatalogue& catalogue) const;
        std::vector<std::string_view> get_sort_buses_name(TransportCatalogue& catalogue) const;
        domain::StopInfo StopQuery(TransportCatalogue& catalogue, const std::string_view stopname) const;
        domain::BusInfo BusQuery(TransportCatalogue& catalogue, std::string_view busname) const;
        json::Node MakeNodeStop(int id_result, StopInfo stop_info);
        json::Node MakeNodeBus(int id_result, BusInfo bus_info);
        json::Node ExecuteRenderMap(int request_id, TransportCatalogue& catalogue, Render_settings render_settings);
        const Document& GetDocument();
        private:
        Document out_;
    };
}