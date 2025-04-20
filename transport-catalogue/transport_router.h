#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

#include <memory>

namespace route
{
    struct RoutingSettings
    {
        int bus_wait_time = 0;
        double bus_velocity = 0.0;
    };

    struct RouteWeight
    {
        bool is_stop = true;
        std::string_view name;
        double route_time = 0.0;
        int span_count = 0;

        bool operator<(const RouteWeight &other) const
        {
            return route_time < other.route_time;
        }

        bool operator>(const RouteWeight &other) const
        {
            return route_time > other.route_time;
        }

        RouteWeight operator+(const RouteWeight &other) const
        {
            return {is_stop, name, route_time + other.route_time, span_count};
        }
    };

    class Router
    {
    public:
        using RoutingSettings = route::RoutingSettings;
        using TransportCatalogue = transport_catalogue::TransportCatalogue;
        explicit Router(const RoutingSettings &settings, const TransportCatalogue &catalogue) : settings_(settings),
                                                                                                graph_(2 * catalogue.GetStops().size())
        {
            BuildStopsGraph(catalogue);
            BuildBusesGraph(catalogue);
            router_ = std::make_unique<graph::Router<RouteWeight>>(graph::Router(graph_));
        };
        std::optional<std::pair<graph::Router<RouteWeight>::RouteInfo,
                                std::vector<RouteWeight>>>
        FindRoute(std::string_view from, std::string_view to) const;

    private:
        const RoutingSettings &settings_;
        constexpr static double TRANSLATE_TO_M_MIN = 1000.0 / 60.0;
        void BuildStopsGraph(const TransportCatalogue &catalogue);
        void BuildBusesGraph(const TransportCatalogue &catalogue);
        graph::DirectedWeightedGraph<RouteWeight> graph_;
        std::unordered_map<std::string_view, domain::StopId> stop_id_;
        std::unique_ptr<graph::Router<RouteWeight>> router_ = nullptr;
    };
}