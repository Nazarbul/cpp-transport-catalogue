#pragma once

#include <deque>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <map>
#include <optional>

#include "geo.h"
#include "domain.h"

namespace transport_catalogue
{

    struct DistanceHasher
    {
        size_t operator()(const std::pair<const domain::Stop *, const domain::Stop *> distances) const
        {
            return hasher_p_(distances.first) + 42 * hasher_p_(distances.second);
        }

    private:
        std::hash<const domain::Stop *> hasher_p_;
    };

    class TransportCatalogue
    {
    public:
        void AddStop(const domain::Stop &stop);
        void AddBus(const domain::Bus &bus);
        domain::Bus *FindBus(std::string_view bus) const;
        domain::Stop *FindStop(const std::string_view stop) const;
        void SetDistances(const std::string_view to, const std::string_view from, const double distance);
        const std::map<std::string_view, domain::Bus *> &GetBuses() const;
        const std::map<std::string_view, domain::Stop *> &GetStops() const;
        double GetDistances(const domain::Stop *from, const domain::Stop *to) const;
        std::optional<domain::BusInfo> BusInfo(const std::string_view busname) const;

    private:
        size_t GetUniqueStops(std::string_view busname) const;
        double GetLengthRoute(std::string_view busname) const;
        std::deque<domain::Stop> stops_;
        std::deque<domain::Bus> buses_;
        std::map<std::string_view, domain::Stop *> stopname_to_stops;
        std::map<std::string_view, domain::Bus *> busname_to_buses;
        std::unordered_map<std::string_view, std::set<domain::Bus *>> bus_to_stops;
        std::unordered_map<std::pair<const domain::Stop *, const domain::Stop *>, double, DistanceHasher> distances;
    };
}