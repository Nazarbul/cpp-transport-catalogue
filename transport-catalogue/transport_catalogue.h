#pragma once

#include <deque>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

namespace transport_catalogue {
    struct Stop {
        std::string name_;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string name_;
        std::vector<Stop*> stops_;
    };

    struct BusInfo {
        std::string name_;
        size_t count_all_stops;
        size_t count_unique_stops;
        double route_length;
    };

    struct StopInfo {
        std::string name_;
        const std::set<Bus*>& buses_;
    };

    class TransportCatalogue {
    public:
        void AddStop(const Stop& stop);
        void AddBus(const Bus& bus);
        Bus* FindBus(std::string_view bus) const;
        const Stop* FindStop(std::string_view stop) const;
        BusInfo GetBusInfo(const std::string_view stopname) const;
        const std::set<Bus*>& GetStopInfo(const std::string_view stopname) const;
    private:
        int GetUniqueStops(std::string_view busname) const;
        double GetLengthRoute(std::string_view busname) const;
        std::deque <Stop> stops_;
        std::deque <Bus> buses_;
        std::unordered_map<std::string_view, Stop*> stopname_to_stops;
        std::unordered_map<std::string_view, Bus*> busname_to_buses;
        std::unordered_map<std::string_view, std::set<Bus*>> bus_to_stops;
    };
}