#include "transport_catalogue.h"

#include <execution>

namespace transport_catalogue {

    void TransportCatalogue::AddStop(const Stop& stop) {
        stops_.push_back(stop);
        Stop* ptr_stop = &stops_[stops_.size() - 1];
        stopname_to_stops[ptr_stop->name_] = ptr_stop;
        
    }

    void TransportCatalogue::AddBus(const Bus& bus) {
        buses_.push_back(bus);
        Bus* bus_ptr = &buses_[buses_.size() - 1];
        busname_to_buses[bus_ptr->name_] = bus_ptr;
        for (const auto stop : bus_ptr->stops_) {
            bus_to_stops[stop->name_].insert(bus_ptr);
        }
    }

    Stop* TransportCatalogue::FindStop(const std::string_view stop_name) const {
        auto result = stopname_to_stops.find(stop_name);
        return result == stopname_to_stops.end() ? nullptr : result->second;
    }

    Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
        auto result = busname_to_buses.find(bus_name);
        return result == busname_to_buses.end() ? nullptr : result->second;

    }

    BusInfo TransportCatalogue::GetBusInfo(std::string_view busname) const {
        BusInfo result;
        auto it = busname_to_buses.find(busname);
        if( it != busname_to_buses.end()){
            const Bus& bus = *(it->second);
            result.name_=busname;
            result.count_all_stops = bus.stops_.size();
            result.count_unique_stops = GetUniqueStops(busname);
            result.geo_length = GetLengthRoute(busname);
            result.route_length = GetDistances(busname);
            result.courvature = static_cast<double>(result.route_length)/result.geo_length;
        }
        return result;
    }

    std::set<std::string_view> TransportCatalogue::GetStopInfo(std::string_view stopname) const{
        auto result = bus_to_stops.find(stopname);
        std::set <std::string_view> busname;
        if(result != bus_to_stops.end()){
        const auto& buses = result->second;
        for(const auto bus : buses){
            busname.insert(bus->name_);
        }
        }
        return busname;
    }

    size_t TransportCatalogue::GetUniqueStops(std::string_view busname) const {
        std::unordered_set<const Stop*> unique_stops;
        for (const auto stop : busname_to_buses.at(busname)->stops_) {
            unique_stops.insert(stop);
        }
        return unique_stops.size();
    }

    double TransportCatalogue::GetLengthRoute(std::string_view busname) const {
        return transform_reduce(std::next(busname_to_buses.at(busname)->stops_.begin()), 
        busname_to_buses.at(busname)->stops_.end(), 
        busname_to_buses.at(busname)->stops_.begin(),
        0.0, 
        std::plus<>{}, 
        [](const Stop* lhs, const Stop* rhs) 
            {
                return geo::ComputeDistance(lhs->coordinates, rhs->coordinates);
            });
    }



    void TransportCatalogue::SetDistances(const std::string_view from, const std::string_view to, const double distance){
        Stop* ptr_stop_from = FindStop(from);
        Stop* ptr_stop_to = FindStop(to);
        if(ptr_stop_from != nullptr && ptr_stop_to != nullptr){
            Distances.emplace(std::pair<const Stop*, const Stop*>(ptr_stop_from, ptr_stop_to), distance);
        }
    }

    size_t TransportCatalogue::GetDistanceBetweenStops(const Stop* from, const Stop* to)const{
        std::pair<const Stop*, const Stop*> stops = {from, to};
        auto result = Distances.find(stops);
        if (result != Distances.end()){
            return result->second;
        }

       stops = {to, from};
       result = Distances.find(stops);
       if(result!= Distances.end()){
        return result -> second;
       }
        return 0;
    }

    double TransportCatalogue::GetDistances(const std::string_view busname) const {
        Bus* bus = FindBus(busname);
        size_t size = bus->stops_.size()-1;
        double distance = 0.0;
        for (size_t i =0; i<size; ++i)
        {
            distance+=GetDistanceBetweenStops(bus->stops_[i], bus->stops_[i+1]);
        }
        return distance;
    }
    
}
