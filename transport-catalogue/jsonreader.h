#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "transport_catalogue.h"
#include "json.h"
#include "map_render.h"
#include "request_handler.h"

using namespace json;
using namespace domain;

namespace transport_catalogue
{
    namespace jsonreader
    {

        class JsonReader
        {
        public:
            JsonReader(std::istream &input, TransportCatalogue& catalogue, map_render::RenderSettings& render_settings): document_(json::Load(input)), 
            catalogue_(catalogue), render_settings_(std::move(render_settings)){};
            void FillCatalogue();
            const json::Node GetStatRequest();
            const json::Node GetRenderRequest();
            map_render::MapRender ParseNodeRequest(const Node &node);
            void ExecuteQueries(const Node &stat_requests, request_handler::RequestHandler &request_handler) const;
            
        private:
            Document document_;
            TransportCatalogue& catalogue_;
            map_render::RenderSettings render_settings_;
            Stop FillStop(Node &node);
            Bus FillBus(Node &node);
            void FillStopDistance(Node &node);
            json::Node StopQuery(const Dict &stat_requests, const request_handler::RequestHandler &request_handler) const;
            json::Node BusQuery(const Dict &stat_requests,const request_handler::RequestHandler &request_handler) const;
            json::Node RenderMapQuery(const Dict &stat_requests, request_handler::RequestHandler &request_handler) const;
            void SetBasicSettings(map_render::RenderSettings &render_settings, const Dict &node);
            void SetLabelSettings(map_render::RenderSettings &render_settings, const Dict &node);
            void SetUnderlayerSettings(map_render::RenderSettings &render_settings, const Dict &node);
            void SetColorPaletteSettings(map_render::RenderSettings &render_settings, const Dict &node);
        };

    }
}