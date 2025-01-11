#pragma once

#include <deque>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

#include "geo.h"

namespace transport_catalogue {
    struct Stop {
        std::string name_;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string name_;
        std::vector<const Stop*> stops_;
    };

    struct BusInfo {
        std::string name_;
        size_t count_all_stops;
        size_t count_unique_stops;
        double geo_length;
        double route_length;
        double courvature;
    };

    struct StopInfo {
        std::string name_;
        const std::set<Bus*>& buses_;
    };

    struct DistanceHasher {
        size_t operator()(const std::pair<const Stop*, const Stop*> distances) const{
            return hasher_p_(distances.first) + 42 * hasher_p_(distances.second);
        }
        private:
        std::hash<const Stop*> hasher_p_; 
    };

    class TransportCatalogue {
    public:
        void AddStop(const Stop& stop);
        void AddBus(const Bus& bus);
        Bus* FindBus(std::string_view bus) const;
        Stop* FindStop(const std::string_view stop) const;
        BusInfo GetBusInfo(const std::string_view stopname) const;
        std::set<std::string_view> GetStopInfo(const std::string_view stopname) const;
        void SetDistance(const std::string_view to, const std::string_view from, const double distance);
    private:
        size_t GetUniqueStops(std::string_view busname) const;
        double GetLengthRoute(std::string_view busname) const;
        size_t GetDistanceBetweenStops(const Stop* from, const Stop* to) const;
        double GetDistances(const std::string_view busname) const;
        std::deque <Stop> stops_;
        std::deque <Bus> buses_;
        std::unordered_map<std::string_view, Stop*> stopname_to_stops;
        std::unordered_map<std::string_view, Bus*> busname_to_buses;
        std::unordered_map<std::string_view, std::set<Bus*>> bus_to_stops;
        std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, DistanceHasher> Distances;
    };
}