#include "transport_catalogue.h"

#include <execution>

namespace transport_catalogue {

    void TransportCatalogue::AddStop(const Stop& stop) {
        stops_.push_back(std::move(stop));
        Stop* ptr_stop = &stops_[stops_.size() - 1];
        stopname_to_stops[ptr_stop->name_] = ptr_stop;
    }

    void TransportCatalogue::AddBus(const Bus& bus) {
        buses_.push_back(std::move(bus));
        Bus* bus_ptr = &buses_[buses_.size() - 1];
        busname_to_buses[bus_ptr->name_] = bus_ptr;
        for (const auto stop : bus_ptr->stops_) {
            bus_to_stops[stop->name_].insert(bus_ptr);
        }
    }

    const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
        auto result = stopname_to_stops.find(stop_name);
        return result == stopname_to_stops.end() ? nullptr : result->second;
    }

    Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
        auto result = busname_to_buses.find(bus_name);
        return result == busname_to_buses.end() ? nullptr : result->second;

    }

    BusInfo TransportCatalogue::GetBusInfo(std::string_view busname) const {
        BusInfo result;
        if(busname_to_buses.find(busname) != busname_to_buses.end()){
            result.name_=busname;
            result.count_all_stops = busname_to_buses.at(busname)->stops_.size();
            result.count_unique_stops = GetUniqueStops(busname);
            result.route_length = GetLengthRoute(busname);
        }
        return result;
    }

    const std::set<Bus*>& TransportCatalogue::GetStopInfo(std::string_view stopname) const{
        static const std::set <Bus*> empty_buses;
        auto result = bus_to_stops.find(stopname);
        return result == bus_to_stops.end() ? empty_buses : result->second;
    }

    int TransportCatalogue::GetUniqueStops(std::string_view busname) const {
        std::unordered_set<const Stop*> unique_stops;
        for (const auto stop : busname_to_buses.at(busname)->stops_) {
            unique_stops.insert(stop);
        }
        return int(unique_stops.size());
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
}