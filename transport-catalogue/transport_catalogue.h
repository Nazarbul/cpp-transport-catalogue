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

    class TransportCatalogue {
    public:
        void AddStop(Stop&& stop);
        void AddBus(Bus&& bus);
        Bus* FindBus(std::string_view bus) const;
        Stop* FindStop(std::string_view stop) const;
        std::set <std::string_view> GetBusInfo(const std::string_view stopname) const;
        int GetUniqueStops(const Bus* Bus) const;
        double GetLengthRoute(const Bus* Bus) const;
    private:
        std::deque <Stop> stops_;
        std::deque <Bus> buses_;
        std::unordered_map<std::string_view, Stop*> stopname_to_stops;
        std::unordered_map<std::string_view, Bus*> busname_to_buses;
        std::unordered_map<std::string_view, std::unordered_set<Bus*>> bus_to_stops;
    };
}