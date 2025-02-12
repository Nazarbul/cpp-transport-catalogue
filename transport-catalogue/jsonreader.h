#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "transport_catalogue.h"
#include "json.h"
#include "map_render.h"

using namespace json;
using namespace domain;

namespace transport_catalogue {
    namespace jsonreader {

        class JsonReader{
            public:
            JsonReader() = default;
            JsonReader(Document document) : document_(document){};
            JsonReader(std::istream& input) : document_(json::Load(input)){};
            Stop ParseNodeStop(Node& node);
            Bus ParseNodeBus(Node& node, TransportCatalogue& catalogue);
            void ParseNodeDistance(Node& node, TransportCatalogue& catalogue);
            void ParseNodeBase(const Node& node, TransportCatalogue& catalogue);
            void ParseNodeStat(const Node& node, std::vector<StatRequest>& stat_request);
            void ParseNodeRequest(const Node& node, map_render::Render_settings& render_settings);
            void ParseNode(const Node& node, TransportCatalogue& catalogue, map_render::Render_settings& render_settings,
            std::vector<StatRequest>& stat_request);
            void Parse(TransportCatalogue& catalogue, map_render::Render_settings& render_settings,
            std::vector<StatRequest>& stat_request);
            const Document& get_document() const;
            private:
            Document document_;
        };

    }
}