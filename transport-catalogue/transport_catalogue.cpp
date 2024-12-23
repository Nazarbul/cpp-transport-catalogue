#include "transport_catalogue.h"

#include <execution>

namespace transport_catalogue {

    void TransportCatalogue::AddStop(Stop&& stop) {
        stops_.push_back(std::move(stop));
        Stop* ptr_stop = &stops_[stops_.size() - 1];
        stopname_to_stops[ptr_stop->name_] = ptr_stop;
    }

    void TransportCatalogue::AddBus(Bus&& bus) {
        buses_.push_back(std::move(bus));
        Bus* bus_ptr = &buses_[buses_.size() - 1];
        busname_to_buses[bus_ptr->name_] = bus_ptr;
        for (const auto stop : bus_ptr->stops_) {
            bus_to_stops[stop->name_].insert(bus_ptr);
        }
    }

    Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
        auto result = stopname_to_stops.find(stop_name);
        return result == stopname_to_stops.end() ? nullptr : result->second;
    }

    Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
        auto result = busname_to_buses.find(bus_name);
        return result == busname_to_buses.end() ? nullptr : result->second;

    }

    std::set <std::string_view> TransportCatalogue::GetBusInfo(std::string_view stopname) const {
        auto result = bus_to_stops.find(stopname);
        std::set<std::string_view> busname;
        if (result != bus_to_stops.end()) {
            const auto& buses = result->second;
            for (const auto bus : buses) {
                busname.insert(bus->name_);
            }
        }
        return busname;
    }

    int TransportCatalogue::GetUniqueStops(const Bus* bus) const {
        std::unordered_set<const Stop*> unique_stops;
        for (const auto stop : bus->stops_) {
            unique_stops.insert(stop);
        }
        return int(unique_stops.size());
    }

    double TransportCatalogue::GetLengthRoute(const Bus* bus) const {
        return transform_reduce(std::next(bus->stops_.begin()), bus->stops_.end(), 
        bus->stops_.begin(), 
        0.0, 
        std::plus<>{}, 
        [](const Stop* lhs, const Stop* rhs) 
            {
                return geo::ComputeDistance(lhs->coordinates, rhs->coordinates);
            });
    }
}