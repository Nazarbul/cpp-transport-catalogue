#include "jsonreader.h"

using namespace std::literals;
namespace transport_catalogue
{
    namespace jsonreader
    {
        domain::Stop JsonReader::FillStop(json::Node &node)
        {
            Dict stop_node;
            Stop stop;
            if (node.IsMap())
            {
                try
                {
                    stop_node = node.AsMap();
                    stop.name_ = stop_node.at("name").AsString();
                    stop.coordinates = {stop_node.at("latitude").AsDouble(), stop_node.at("longitude").AsDouble()};
                }
                catch (const std::out_of_range &)
                {
                    throw std::runtime_error("Error: invalid stop");
                }
            }
            return stop;
        }

        void JsonReader::FillStopDistance(json::Node &node)
        {
            Dict stop_node;
            Dict stop_road_map;
            std::string begin_name;
            std::string last_name;
            int distance = 0;
            if (node.IsMap())
            {
                stop_node = node.AsMap();
                begin_name = stop_node.at("name").AsString();
                try
                {
                    stop_road_map = stop_node.at("road_distances").AsMap();
                    for (auto [key, value] : stop_road_map)
                    {
                        last_name = key;
                        distance = value.AsInt();
                        catalogue_.SetDistances(begin_name, last_name, distance);
                    }
                }
                catch (const std::out_of_range &)
                {
                    throw std::runtime_error("Error: Road invalide");
                }
            }
        }

        domain::Bus JsonReader::FillBus(Node &node)
        {
            Dict bus_node;
            Bus bus;
            Array stops_node;
            if (node.IsMap())
            {
                bus_node = node.AsMap();
                bus.name_ = bus_node.at("name").AsString();
                bus.is_roundtrip = bus_node.at("is_roundtrip").AsBool();
                try
                {
                    stops_node = bus_node.at("stops").AsArray();
                    for (Node stop_node : stops_node)
                    {
                        bus.stops_.push_back(catalogue_.FindStop(stop_node.AsString()));
                    }
                    if (!bus.is_roundtrip)
                    {
                        size_t size = bus.stops_.size() - 1;
                        for (size_t i = size; i > 0; --i)
                        {
                            bus.stops_.push_back(bus.stops_[i - 1]);
                        }
                    }
                }
                catch (const std::out_of_range &)
                {
                    throw std::runtime_error("Error: base_requests: bus: stops is empty");
                }
            }
            return bus;
        }

        void JsonReader::ParseBaseRequest(Array &base_request, std::vector<Node> &buses, std::vector<Node> &stops)
        {
            Dict req_map;
            Node req_node;
            for (auto &node : base_request)
            {
                if (node.IsMap())
                {
                    req_map = node.AsMap();
                    try
                    {
                        req_node = req_map.at("type").AsString();
                        if (req_node.IsString())
                        {
                            if (req_node.AsString() == "Bus")
                            {
                                buses.push_back(req_map);
                            }
                            else if (req_node.AsString() == "Stop")
                            {
                                stops.push_back(req_map);
                            }
                        }
                    }
                    catch (const std::out_of_range &)
                    {
                        throw std::runtime_error("Error: base_requests not have type value");
                    }
                }
            }
        }

        void JsonReader::FillCatalogue()
        {
            Array base_request;
            std::vector<Node> buses;
            std::vector<Node> stops;
            if (document_.GetRoot().AsMap().count("base_requests"))
            {
                base_request = document_.GetRoot().AsMap().at("base_requests").AsArray();
                ParseBaseRequest(base_request, buses, stops);
                for (auto stop : stops)
                {
                    catalogue_.AddStop(FillStop(stop));
                }
                for (auto stop : stops)
                {
                    FillStopDistance(stop);
                }
                for (auto bus : buses)
                {
                    catalogue_.AddBus(FillBus(bus));
                }
            }
        }

        const json::Node JsonReader::GetStatRequest()
        {
            if (!document_.GetRoot().AsMap().count("stat_requests"))
            {
                return nullptr;
            }
            return document_.GetRoot().AsMap().at("stat_requests");
        }

        const json::Node JsonReader::GetRenderRequest()
        {
            if (!document_.GetRoot().AsMap().count("render_settings"))
            {
                return nullptr;
            }
            return document_.GetRoot().AsMap().at("render_settings");
        }

        const json::Node JsonReader::GetRoutingRequest()
        {
            if (!document_.GetRoot().AsMap().count("routing_settings"))
            {
                return nullptr;
            }
            return document_.GetRoot().AsMap().at("routing_settings");
        }

        route::RoutingSettings JsonReader::FillRouting(const Node &routing_settings) const
        {
            const auto &wait_time = routing_settings.AsMap().at("bus_wait_time").AsInt();
            const auto &velocity = routing_settings.AsMap().at("bus_velocity").AsDouble();
            static route::RoutingSettings settings{wait_time, velocity};
            return settings;
        }

        void JsonReader::SetBasicSettings(map_render::RenderSettings &render_settings, const Dict &root_dict)
        {
            try
            {
                render_settings.width = root_dict.at("width").AsDouble();
                render_settings.height = root_dict.at("height").AsDouble();
                render_settings.padding = root_dict.at("padding").AsDouble();
                render_settings.stop_radius = root_dict.at("stop_radius").AsDouble();
                render_settings.line_width = root_dict.at("line_width").AsDouble();
                render_settings.underlayer_width = root_dict.at("underlayer_width").AsDouble();
            }
            catch (const std::out_of_range &)
            {
                throw std::runtime_error("basic render_settings is empty");
            }
        }

        void JsonReader::SetLabelSettings(map_render::RenderSettings &render_settings, const Dict &root_dict)
        {
            Array bus_label_offset;
            Array stop_label_offset;
            try
            {
                render_settings.bus_label_font_size = static_cast<uint32_t>(root_dict.at("bus_label_font_size").AsDouble());
                if (root_dict.at("bus_label_offset").IsArray())
                {
                    bus_label_offset = root_dict.at("bus_label_offset").AsArray();
                    render_settings.bus_label_offset = svg::Point{bus_label_offset[0].AsDouble(),
                                                                  bus_label_offset[1].AsDouble()};
                }
                render_settings.stop_label_font_size = static_cast<uint32_t>(root_dict.at("stop_label_font_size").AsDouble());
                if (root_dict.at("stop_label_offset").IsArray())
                {
                    stop_label_offset = root_dict.at("stop_label_offset").AsArray();
                    render_settings.stop_label_offset = svg::Point{stop_label_offset[0].AsDouble(),
                                                                   stop_label_offset[1].AsDouble()};
                }
            }
            catch (const std::out_of_range &)
            {
                throw std::runtime_error("label render_settings is empty");
            }
        }

        void JsonReader::SetUnderlayerSettings(map_render::RenderSettings &render_settings, const Dict &root_dict)
        {
            Array underlayer_color;
            try
            {
                if (root_dict.at("underlayer_color").IsString())
                {
                    render_settings.underlayer_color = svg::Color(root_dict.at("underlayer_color").AsString());
                }
                else if (root_dict.at("underlayer_color").IsArray())
                {
                    underlayer_color = root_dict.at("underlayer_color").AsArray();
                    if (underlayer_color.size() == 4)
                    {
                        render_settings.underlayer_color = svg::Rgba{
                            static_cast<uint8_t>(underlayer_color[0].AsInt()),
                            static_cast<uint8_t>(underlayer_color[1].AsInt()),
                            static_cast<uint8_t>(underlayer_color[2].AsInt()),
                            underlayer_color[3].AsDouble()};
                    }
                    if (underlayer_color.size() == 3)
                    {
                        render_settings.underlayer_color = svg::Rgb{
                            static_cast<uint8_t>(underlayer_color[0].AsInt()),
                            static_cast<uint8_t>(underlayer_color[1].AsInt()),
                            static_cast<uint8_t>(underlayer_color[2].AsInt())};
                    }
                }
            }
            catch (const std::out_of_range &)
            {
                throw std::runtime_error("underlayer render_settings is empty");
            }
        }

        void JsonReader::SetColorPaletteSettings(map_render::RenderSettings &render_settings, const Dict &root_dict)
        {
            Array color_palette;
            Array color_array;
            try
            {
                if (root_dict.at("color_palette").IsArray())
                {
                    color_palette = root_dict.at("color_palette").AsArray();
                    for (Node &color : color_palette)
                    {
                        if (color.IsString())
                        {
                            render_settings.color_palette.push_back(color.AsString());
                        }
                        else if (color.IsArray())
                        {
                            color_array = color.AsArray();
                            if (color_array.size() == 3)
                            {
                                render_settings.color_palette.push_back(svg::Rgb(
                                    color_array[0].AsInt(),
                                    color_array[1].AsInt(),
                                    color_array[2].AsInt()));
                            }
                            else if (color_array.size() == 4)
                            {
                                render_settings.color_palette.push_back(svg::Rgba(
                                    color_array[0].AsInt(),
                                    color_array[1].AsInt(),
                                    color_array[2].AsInt(),
                                    color_array[3].AsDouble()));
                            }
                        }
                    }
                }
            }
            catch (const std::out_of_range &)
            {
                throw std::runtime_error("color palette render_settings is empty");
            }
        }

        map_render::MapRender JsonReader::ParseNodeRequest(const Node &node)
        {
            Dict root_dict;
            if (node.IsMap())
            {
                root_dict = node.AsMap();
                SetBasicSettings(render_settings_, root_dict);
                SetLabelSettings(render_settings_, root_dict);
                SetUnderlayerSettings(render_settings_, root_dict);
                SetColorPaletteSettings(render_settings_, root_dict);
            }
            return render_settings_;
        }

        void JsonReader::ExecuteQueries(const Node &stat_requests,
                                        request_handler::RequestHandler &request_handler) const
        {
            Array result_request;
            Dict stat_request;
            for (const auto &request : stat_requests.AsArray())
            {
                stat_request = request.AsMap();
                if (stat_request.at("type").AsString() == "Stop")
                {
                    result_request.push_back(StopQuery(stat_request, request_handler).AsMap());
                }
                else if (stat_request.at("type").AsString() == "Bus")
                {
                    result_request.push_back(BusQuery(stat_request, request_handler).AsMap());
                }
                else if (stat_request.at("type").AsString() == "Map")
                {
                    result_request.push_back(RenderMapQuery(stat_request, request_handler).AsMap());
                }
                else if (stat_request.at("type").AsString() == "Route")
                {
                    result_request.push_back(RouteQuery(stat_request, request_handler));
                }
            }
            json::Print(Document(result_request), std::cout);
        }

        json::Node JsonReader::StopQuery(const Dict &stat_request, const request_handler::RequestHandler &request_handler) const
        {
            json::Node result;
            Array buses_;
            std::string stop_name = stat_request.at("name").AsString();
            auto stop = request_handler.SearchStopName(stop_name);
            std::string str_not_found = "not found";
            const auto result_id = stat_request.at("id").AsInt();
            if (stop)
            {
                auto buses = request_handler.GetBusesOnStop(stop_name);
                if (buses.size() > 0)
                {
                    for (const auto &bus : buses)
                    {
                        buses_.push_back(bus);
                    }
                }
                std::sort(buses_.begin(), buses_.end(), [](const json::Node &lhs, json::Node &rhs)
                          { return lhs.AsString() < rhs.AsString(); });
                result = json::Builder{}.StartDict().Key("request_id").Value(result_id).Key("buses").Value(buses_).EndDict().Build();
            }
            else
            {
                result = json::Builder{}.StartDict().Key("request_id").Value(result_id).Key("error_message").Value(str_not_found).EndDict().Build();
            }
            return result;
        }

        json::Node JsonReader::BusQuery(const Dict &stat_request,
                                        const request_handler::RequestHandler &request_handler) const
        {
            Node result;
            std::string bus_name = stat_request.at("name").AsString();
            auto bus = request_handler.SearchBusNumber(bus_name);
            const auto result_id = stat_request.at("id").AsInt();
            std::string str_not_found = "not found";
            if (bus)
            {
                const auto &bus_info = request_handler.GetBusInfo(bus_name);
                result = Builder{}.StartDict().Key("request_id").Value(result_id).Key("curvature").Value(bus_info->courvature).Key("route_length").Value(bus_info->route_length).Key("stop_count").Value(static_cast<int>(bus_info->count_all_stops)).Key("unique_stop_count").Value(static_cast<int>(bus_info->count_unique_stops)).EndDict().Build();
            }
            else
            {
                result = Builder{}.StartDict().Key("request_id").Value(result_id).Key("error_message").Value(str_not_found).EndDict().Build();
            }
            return result;
        }
        json::Node JsonReader::RenderMapQuery(const Dict &stat_requests, request_handler::RequestHandler &request_handler) const
        {
            Node result;
            std::ostringstream out;
            const int id = stat_requests.at("id").AsInt();
            auto map = request_handler.RenderMap();
            map.Render(out);
            result = Builder{}.StartDict().Key("request_id").Value(id).Key("map").Value(out.str()).EndDict().Build();
            return result;
        }

        json::Node JsonReader::RouteQuery(const Dict &stat_requests, request_handler::RequestHandler &request_handler) const
        {
            Builder result;
            const int id = stat_requests.at("id").AsInt();
            auto stop_from = stat_requests.at("from").AsString();
            auto stop_to = stat_requests.at("to").AsString();
            std::string str_not_found = "not found";
            std::optional<std::pair<graph::Router<route::RouteWeight>::RouteInfo,
                                    std::vector<route::RouteWeight>>>
                routing = request_handler.GetRouting(stop_from, stop_to);
            if (!routing)
            {
                result.StartDict().Key("request_id").Value(id).Key("error_message").Value(str_not_found).
                EndDict();
                return result.Build();
            }
                result.StartDict().Key("request_id").Value(id).Key("total_time").
                Value(routing.value().first.weight.route_time).Key("items").StartArray();
            
            for (const route::RouteWeight &weight : routing.value().second)
            {
                if (weight.is_stop)
                {
                    result. StartDict().Key("type").Value("Wait").Key("stop_name").
                    Value(std::string(weight.name)).Key("time"s).Value(weight.route_time).
                    EndDict();
                }
                else
                {
                    result.StartDict().Key("type"s).Value("Bus"s).Key("bus"s).
                    Value(std::string(weight.name)).Key("span_count"s).Value(weight.span_count).
                    Key("time"s).Value(weight.route_time).EndDict();
                }
            }
                result.EndArray().EndDict();
            return result.Build();
        }
    }
}
