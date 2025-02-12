#include "transport_catalogue.h"

#include <execution>

namespace transport_catalogue {

    void TransportCatalogue::AddStop(const domain::Stop& stop) {
        stops_.push_back(stop);
        domain::Stop* ptr_stop = &stops_[stops_.size() - 1];
        stopname_to_stops[ptr_stop->name_] = ptr_stop;

        
    }

    void TransportCatalogue::AddBus(const domain::Bus& bus) {
        buses_.push_back(bus);
        domain::Bus* bus_ptr = &buses_[buses_.size() - 1];
        busname_to_buses[bus_ptr->name_] = bus_ptr;
        for (auto stop : bus_ptr->stops_) {
            bus_to_stops[stop->name_].insert(bus_ptr);
            stop->buses.push_back(bus_ptr);
        }
    }

    domain::Stop* TransportCatalogue::FindStop(const std::string_view stop_name) {
        if(stopname_to_stops.empty()){
            return nullptr;
        }
        try{
            return stopname_to_stops.at(stop_name);
        }
        catch(const std::out_of_range &e){
            return nullptr;
        }
    }

    domain::Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
        auto result = busname_to_buses.find(bus_name);
        return result == busname_to_buses.end() ? nullptr : result->second;

    }

    std::unordered_set<const domain::Bus*> TransportCatalogue::GetBusesToStops(std::string_view stopname){
        std::unordered_set<const domain::Bus*> result;
        try{
        for(const auto bus : bus_to_stops.at(stopname)){
            result.insert(bus);
        }
        return result;
        }
        catch(const std::out_of_range &e){
            return {};
        }
    }

    size_t TransportCatalogue::GetUniqueStops(std::string_view busname) const {
        std::unordered_set<const domain::Stop*> unique_stops;
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
        [](const domain::Stop* lhs, const domain::Stop* rhs) 
            {
                return geo::ComputeDistance(lhs->coordinates, rhs->coordinates);
            });
    }



    void TransportCatalogue::SetDistances(const std::string_view from, const std::string_view to, const double distance){
        domain::Stop* ptr_stop_from = FindStop(from);
        domain::Stop* ptr_stop_to = FindStop(to);
        if(ptr_stop_from != nullptr && ptr_stop_to != nullptr){
            Distances.emplace(std::pair<const domain::Stop*, const domain::Stop*>(ptr_stop_from, ptr_stop_to), distance);
        }
    }

    size_t TransportCatalogue::GetDistanceBetweenStops(const domain::Stop* from, const domain::Stop* to)const{
        std::pair<const domain::Stop*, const domain::Stop*> stops = {from, to};
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
        domain::Bus* bus = FindBus(busname);
        size_t size = bus->stops_.size()-1;
        double distance = 0.0;
        for (size_t i =0; i<size; ++i)
        {
            distance+=GetDistanceBetweenStops(bus->stops_[i], bus->stops_[i+1]);
        }
        return distance;
    }

    std::unordered_map<std::string_view, domain::Bus*> TransportCatalogue::GetBuses() const{
        return busname_to_buses;
    }
    
    std::unordered_map<std::string_view, domain::Stop*> TransportCatalogue::GetStops() const {
        return stopname_to_stops;
    }
}