#include "transport_catalogue.h"

#include <numeric>

namespace transport_catalogue
{

    void TransportCatalogue::AddStop(const domain::Stop &stop)
    {
        stops_.push_back(stop);
        domain::Stop *ptr_stop = &stops_[stops_.size() - 1];
        stopname_to_stops[ptr_stop->name_] = ptr_stop;
    }

    void TransportCatalogue::AddBus(const domain::Bus &bus)
    {
        buses_.push_back(bus);
        domain::Bus *bus_ptr = &buses_[buses_.size() - 1];
        busname_to_buses[bus_ptr->name_] = bus_ptr;
        for (auto stop : bus_ptr->stops_)
        {
            bus_to_stops[stop->name_].insert(bus_ptr);
            stop->buses.insert(bus_ptr->name_);
        }
    }

    domain::Stop *TransportCatalogue::FindStop(const std::string_view stop_name) const
    {
        auto result = stopname_to_stops.find(stop_name);
        return result == stopname_to_stops.end() ? nullptr : result->second;
    }

    domain::Bus *TransportCatalogue::FindBus(std::string_view bus_name) const
    {
        auto result = busname_to_buses.find(bus_name);
        return result == busname_to_buses.end() ? nullptr : result->second;
    }

    size_t TransportCatalogue::GetUniqueStops(std::string_view busname) const
    {
        std::unordered_set<const domain::Stop *> unique_stops;
        for (const auto stop : busname_to_buses.at(busname)->stops_)
        {
            unique_stops.insert(stop);
        }
        return unique_stops.size();
    }

    double TransportCatalogue::GetLengthRoute(std::string_view busname) const
    {
        return transform_reduce(std::next(busname_to_buses.at(busname)->stops_.begin()),
                                busname_to_buses.at(busname)->stops_.end(),
                                busname_to_buses.at(busname)->stops_.begin(),
                                0.0,
                                std::plus<>{},
                                [](const domain::Stop *lhs, const domain::Stop *rhs)
                                {
                                    return geo::ComputeDistance(lhs->coordinates, rhs->coordinates);
                                });
    }

    void TransportCatalogue::SetDistances(const std::string_view from, const std::string_view to, const double distance)
    {
        const domain::Stop *ptr_stop_from = FindStop(from);
        const domain::Stop *ptr_stop_to = FindStop(to);
        if (ptr_stop_from != nullptr && ptr_stop_to != nullptr)
        {
            distances.emplace(std::pair<const domain::Stop *, const domain::Stop *>(ptr_stop_from, ptr_stop_to), distance);
        }
    }

    double TransportCatalogue::GetDistances(const domain::Stop *from, const domain::Stop *to) const
    {
        std::pair<const domain::Stop *, const domain::Stop *> stops = {from, to};
        auto result = distances.find(stops);
        if (result != distances.end())
        {
            return result->second;
        }

        stops = {to, from};
        result = distances.find(stops);
        if (result != distances.end())
        {
            return result->second;
        }
        return 0.0;
    }

    const std::map<std::string_view, domain::Bus *> &TransportCatalogue::GetBuses() const
    {
        return busname_to_buses;
    }

    const std::map<std::string_view, domain::Stop *> &TransportCatalogue::GetStops() const
    {
        return stopname_to_stops;
    }

    std::optional<domain::BusInfo> TransportCatalogue::BusInfo(const std::string_view busname) const
    {
        domain::BusInfo bus_info;
        const auto bus = FindBus(busname);
        if (bus)
        {
            bus_info.name_ = busname;
            bus_info.count_unique_stops = GetUniqueStops(busname);
            bus_info.count_all_stops = bus->stops_.size();
            std::vector<domain::Stop *> &stops = bus->stops_;
            double route_length = 0.0;
            for (size_t i = 0; i < bus_info.count_all_stops - 1; ++i)
            {
                route_length += GetDistances(stops[i], stops[i + 1]);
            }
            bus_info.route_length = route_length;
            bus_info.geo_length = GetLengthRoute(busname);
            bus_info.courvature = bus_info.route_length / bus_info.geo_length;
            return bus_info;
        }
        return std::nullopt;
    }
}