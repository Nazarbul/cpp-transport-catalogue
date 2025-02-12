#include "request_handler.h"

using namespace request_handler;

std::vector <geo::Coordinates> RequestHandler::get_stops_coordinates(TransportCatalogue& catalogue_) const {
    std::vector<geo::Coordinates> stop_coordinates;
    auto buses = catalogue_.GetBuses();
    for(auto& [busname, bus] : buses){
        for(auto& stop : bus->stops_){
            geo::Coordinates coordinates;
            coordinates.lat = stop->coordinates.lat;
            coordinates.lng = stop->coordinates.lng;
            stop_coordinates.push_back({coordinates.lat, coordinates.lng});
        }
    }
    return stop_coordinates;
}

std::vector<std::string_view> RequestHandler::get_sort_buses_name(TransportCatalogue& catalogue) const {
    std::vector<std::string_view> buses_;
    auto buses = catalogue.GetBuses();
    if(buses.size() > 0){
        for(auto& [busname, bus] : buses){
            buses_.push_back(busname);
        }
        std::sort(buses_.begin(), buses_.end());
    }
    else
    {
        return {};
    }
        return buses_;
}


void RequestHandler::render_map(map_render::MapRender& Map_render, TransportCatalogue& catalogue_) const {
    std::vector<Stop*> sort_stops;
    std::vector<std::pair<domain::Bus*, int>> buses_palette;
    int palette_size = 0;
    int palette_index = 0;
    palette_size = Map_render.get_palette_size();
    if(palette_size == 0){
        std::cout<<"color palette is empty"s;
        return;
    }
    auto buses = catalogue_.GetBuses();
    if(buses.size() > 0){
        for(const auto bus : get_sort_buses_name (catalogue_)){
            domain::Bus* bus_info = catalogue_.FindBus(bus);
            if(bus_info){
                if(bus_info->stops_.size()>0){
                    buses_palette.push_back({bus_info, palette_index});
                    palette_index = (palette_index + 1) % palette_size;
                }
            }
        }
        if(buses_palette.size()>0){
                Map_render.add_line(buses_palette);
                Map_render.add_buses_name(buses_palette); 
            }
        }
    std::vector<std::string_view> stopnames;
    auto stops = catalogue_.GetStops();
    if(stops.size() > 0){
        for(const auto& [stop_name, stop] : stops){
            if(stop->buses.size() > 0){
                stopnames.push_back(stop_name);
            }
        }
        std::sort(stopnames.begin(), stopnames.end());

        domain::Stop* stop;
        for(const auto& stopname : stopnames){
            stop = catalogue_.FindStop(stopname);
            if(stop){
                sort_stops.push_back(stop);
            }
        }
        if(sort_stops.size() > 0){
            Map_render.add_stops_circle(sort_stops);
            Map_render.add_stops_name(sort_stops);
        }
    }   
}

    json::Node RequestHandler::MakeNodeStop(int id_result, StopInfo stopinfo){
    Dict result;
    Array buses_;
    std::string str_not_found = "not found";
    if(stopinfo.not_found){
        result.emplace("request_id", Node{id_result});
        result.emplace("error_message", Node{str_not_found});
    }
    else{
        result.emplace("request_id", Node{id_result});
        for (std::string bus : stopinfo.buses_){
            buses_.push_back(Node{bus});
        }
            result.emplace("buses", Node{buses_});
    }
    return Node{result};
}

    json::Node RequestHandler::MakeNodeBus(int id_result, BusInfo bus_info){
    Dict result;
    std::string str_not_found = "not found";
    if(!bus_info.not_found){
        result.emplace("request_id", Node{id_result});
        result.emplace("curvature", Node{bus_info.courvature});
        result.emplace("route_length", Node{bus_info.route_length});
        result.emplace("stop_count", Node{static_cast<int>(bus_info.count_all_stops)});
        result.emplace("unique_stop_count", Node{static_cast<int>(bus_info.count_unique_stops)});
    }
    else {
        result.emplace("request_id", Node{id_result});
        result.emplace("error_message", Node{str_not_found});
    }
    return Node{result};
}

json::Node RequestHandler::ExecuteRenderMap(int request_id, TransportCatalogue& catalogue_, Render_settings render_settings){
    Dict node;
    std::ostringstream out;
    std::string map;
    MapRender Map_render(render_settings);
    Map_render.init_sphere_projector(get_stops_coordinates(catalogue_));
    render_map(Map_render, catalogue_);
    Map_render.GetMapStream(out);
    map = out.str();
    node.emplace("request_id", Node(request_id));
    node.emplace("map", Node(map));
    return Node{node};
}

    domain::StopInfo RequestHandler::StopQuery(TransportCatalogue& catalogue, std::string_view stopname) const{
        domain::StopInfo stop_info;
        auto stop =  catalogue.FindStop(stopname);
        if(stop != nullptr){
            stop_info.name_ = stop->name_;
            stop_info.not_found = false;
            auto buses = catalogue.GetBusesToStops(stopname);
            if(buses.size()>0){
                for(const auto bus : buses){
                 stop_info.buses_.push_back(bus->name_);
                }
                std::sort(stop_info.buses_.begin(), stop_info.buses_.end());
            }
        }else{
            stop_info.name_= stopname;
            stop_info.not_found = true;
        }
        return stop_info;
    }

    domain::BusInfo RequestHandler::BusQuery(TransportCatalogue& catalogue, std::string_view busname) const{
        domain::BusInfo result;
        auto bus = catalogue.FindBus(busname);
        if( bus != nullptr){
            result.name_=bus->name_;
            result.not_found = false;
            result.count_all_stops = bus->stops_.size();
            result.count_unique_stops = catalogue.GetUniqueStops(bus->name_);
            result.geo_length = catalogue.GetLengthRoute(bus->name_);
            result.route_length = catalogue.GetDistances(bus->name_);
            result.courvature = static_cast<double>(result.route_length)/result.geo_length;
        }
        else {
            result.name_ = busname;
            result.not_found = true;
        }
        return result;
    }

    void RequestHandler::execute_queries(TransportCatalogue& catalogue, 
    std::vector<StatRequest>& stat_requests, map_render::Render_settings& render_settings){
        std::vector<Node> result_request;
        for(const auto& stat_request : stat_requests){
            if(stat_request.type == "Stop"){
                result_request.push_back(MakeNodeStop(stat_request.id, StopQuery(catalogue, stat_request.name)));
            }
            else if(stat_request.type == "Bus"){
                result_request.push_back(MakeNodeBus(stat_request.id, BusQuery(catalogue, stat_request.name)));
            }
            else if(stat_request.type == "Map"){
                result_request.push_back(ExecuteRenderMap(stat_request.id, catalogue, render_settings));
            }
        }
        out_ = Document{Node(result_request)};
    }

    const Document& RequestHandler::GetDocument(){
            return out_;
        }




