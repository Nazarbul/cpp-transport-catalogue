#include "jsonreader.h"

using namespace std::literals;
namespace transport_catalogue{
namespace jsonreader {
domain::Stop JsonReader::ParseNodeStop(json::Node& node){
        Dict stop_node;
        Stop stop;
        if(node.IsMap()){
            stop_node = node.AsMap();
            stop.name_= stop_node.at("name").AsString();
            stop.coordinates = {stop_node.at("latitude").AsDouble(), stop_node.at("longitude").AsDouble()};
        }
        return stop;
}

void JsonReader::ParseNodeDistance(json::Node& node, TransportCatalogue& catalogue){
    Dict stop_node;
    Dict stop_road_map;
    std::string begin_name;
    std::string last_name;
    int distance = 0;
    if(node.IsMap()){
        stop_node = node.AsMap();
        begin_name = stop_node.at("name").AsString();
        try{
            stop_road_map = stop_node.at("road_distances").AsMap();
            for (auto [key, value] : stop_road_map){
                last_name = key;
                distance = value.AsInt();
                catalogue.SetDistances(begin_name, last_name, distance);
            }
        }
        catch(...){
            std::cout << "Error: Road invalide" << std::endl;
        }
    }
}

domain::Bus JsonReader::ParseNodeBus(Node& node, TransportCatalogue& catalogue){
    Dict bus_node;
    Bus bus;
    Array stops_node;
    if(node.IsMap()){
        bus_node = node.AsMap();
        bus.name_ = bus_node.at("name").AsString();
        bus.is_roundtrip = bus_node.at("is_roundtrip").AsBool();
        try {
            stops_node = bus_node.at("stops").AsArray();
            for(Node node : stops_node){
                bus.stops_.push_back(catalogue.FindStop(node.AsString()));
            }
            if(!bus.is_roundtrip){
            size_t size = bus.stops_.size()-1;
                for(size_t i = size; i>0; --i){
                bus.stops_.push_back(bus.stops_[i-1]);
                }
            }
        }
        catch(...){
            std::cout << "Error: base_requests: bus: stops is empty" << std::endl;
        }
    }
    return bus;
}

void JsonReader::ParseNodeBase(const Node& node, TransportCatalogue& catalogue){
    Array base_request;
    Dict req_map;
    Node req_node;
    std::vector<Node> buses;
    std::vector<Node> stops;
    if(node.IsArray()){
        base_request = node.AsArray();
        for(Node& node : base_request){
            if(node.IsMap()){
                req_map = node.AsMap();
                try{
                    req_node = req_map.at("type");
                    if(req_node.IsString()){
                        if(req_node.AsString() == "Bus"){
                            buses.push_back(req_map);
                        }
                        else if(req_node.AsString() == "Stop"){
                            stops.push_back(req_map);
                        }
                        else{
                            std::cout << "Error: base_requests have bad type";
                        }
                    }
                } catch(...){
                    std::cout << "Error: base_requests not have type value";
                }
            } 
        }
       for(auto stop : stops){
        catalogue.AddStop(ParseNodeStop(stop));
       }
       for(auto stop : stops){
        ParseNodeDistance(stop, catalogue);
       }
       for(auto bus : buses){
        catalogue.AddBus(ParseNodeBus(bus, catalogue));
       }

    }
    else {
        std::cout << "Error: base_requests is not array";
    }
}

void JsonReader::ParseNodeStat(const Node& node, std::vector<StatRequest>& stat_request){
    Dict node_map;
    Array stat_requests;
    StatRequest request;
    if(node.IsArray()){
        stat_requests = node.AsArray();
        for(Node node : stat_requests){
            if(node.IsMap()){
            node_map = node.AsMap();
            request.id = node_map.at("id").AsInt();
            request.type = node_map.at("type").AsString();
            if(request.type != "Map"){
            request.name = node_map.at("name").AsString();
            }
            else{
                request.name = "";
            }
            stat_request.push_back(request);
            }
        }
    }
    else {
        std::cout << "Error: base_requests is not array";
    }
   
}

void JsonReader::ParseNodeRequest(const Node& node, map_render::Render_settings& render_settings){
    Dict root_dict;
    Array bus_label_offset;
    Array stop_label_offset;
    Array underlayer_color;
    Array color_palette;
    Array color_array;
    if(node.IsMap()){
        root_dict = node.AsMap();
        try{
            render_settings.width = root_dict.at("width").AsDouble();
            render_settings.height = root_dict.at("height").AsDouble();
            render_settings.padding = root_dict.at("padding").AsDouble();
            render_settings.stop_radius = root_dict.at("stop_radius").AsDouble();
            render_settings.line_width = root_dict.at("line_width").AsDouble();
            render_settings.bus_label_font_size = root_dict.at("bus_label_font_size").AsDouble();
            if(root_dict.at("bus_label_offset").IsArray()){
                bus_label_offset = root_dict.at("bus_label_offset").AsArray();
                render_settings.bus_label_offset = svg::Point{bus_label_offset[0].AsDouble(), 
                bus_label_offset[1].AsDouble()};
            }
            render_settings.stop_label_font_size = root_dict.at("stop_label_font_size").AsDouble();
            if(root_dict.at("stop_label_offset").IsArray()){
                stop_label_offset = root_dict.at("stop_label_offset").AsArray();
                render_settings.stop_label_offset = svg::Point{stop_label_offset[0].AsDouble(), 
                stop_label_offset[1].AsDouble()};
            }
            if(root_dict.at("underlayer_color").IsString()){
                render_settings.underlayer_color = svg::Color(root_dict.at("underlayer_color").AsString());
            }
            else if(root_dict.at("underlayer_color").IsArray()){
                underlayer_color = root_dict.at("underlayer_color").AsArray();
                if(underlayer_color.size() == 4){
                    render_settings.underlayer_color = svg::Rgba{
                        static_cast<uint8_t>(underlayer_color[0].AsInt()),
                        static_cast<uint8_t>(underlayer_color[1].AsInt()), 
                        static_cast<uint8_t>(underlayer_color[2].AsInt()),
                        underlayer_color[3].AsDouble()};
                }
                if(underlayer_color.size() == 3){
                    render_settings.underlayer_color = svg::Rgb{
                    static_cast<uint8_t>(underlayer_color[0].AsInt()),
                    static_cast<uint8_t>(underlayer_color[1].AsInt()), 
                    static_cast<uint8_t>(underlayer_color[2].AsInt())};
                }
            }
            render_settings.underlayer_width = root_dict.at("underlayer_width").AsDouble();
            if(root_dict.at("color_palette").IsArray()){
                color_palette = root_dict.at("color_palette").AsArray();
                for(Node color : color_palette){
                    if(color.IsString()){
                        render_settings.color_palette.push_back(color.AsString());
                    }
                    else if(color.IsArray()){
                        color_array = color.AsArray();
                        if(color_array.size() == 3){
                            render_settings.color_palette.push_back(svg::Rgb
                            {static_cast<uint8_t>(color_array[0].AsInt()),
                            static_cast<uint8_t>(color_array[1].AsInt()), 
                            static_cast<uint8_t>(color_array[2].AsInt())});
                        }
                        else if(color_array.size() == 4){
                            render_settings.color_palette.push_back(svg::Rgba
                            {static_cast<uint8_t>(color_array[0].AsInt()),
                            static_cast<uint8_t>(color_array[1].AsInt()), 
                            static_cast<uint8_t>(color_array[2].AsInt()), 
                            color_array[3].AsDouble()});
                        }
                    }
                }
            }
        } 
        catch(...){
        std::cout << "unable to parse init settings";
        }
    }
    else {
        std::cout << "render_settings is not map";
    }
}

void JsonReader::ParseNode(const Node& node, TransportCatalogue& catalogue,
map_render::Render_settings& render_settings, std::vector<StatRequest>& stat_request){
    Dict root_dict;
    if(node.IsMap()){
        root_dict = node.AsMap();

        try{
            ParseNodeBase(root_dict.at("base_requests"), catalogue);
        } catch(...){
            std::cout << "Error: base_requests is empty";
        }
        try{
            ParseNodeRequest(root_dict.at("render_settings"), render_settings);
        }
        catch(...){
            std::cout << "render_settings is empty";
        }
        try{
            ParseNodeStat(root_dict.at("stat_requests"), stat_request);
        }
        catch(...){
            std::cout << "Error: stat_requests is empty";
        }
    }
    else {
        std::cout << "Error: root is not map";
    }
}

void JsonReader::Parse(TransportCatalogue& catalogue, map_render::Render_settings& render_settings,
std::vector<StatRequest>& stat_request){
    ParseNode(document_.GetRoot(), catalogue, render_settings, stat_request);
}

const Document& JsonReader::get_document() const{
    return document_;
}
}
}
