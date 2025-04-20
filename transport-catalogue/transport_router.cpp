#include "transport_router.h"

namespace route
{

    void Router::BuildStopsGraph(const TransportCatalogue &catalogue)
    {
        domain::StopId stop{0, 1};
        auto &stops = catalogue.GetStops();
        for (const auto &[_, Stop] : stops)
        {
            stop_id_.insert({Stop->name_, stop});
            graph_.AddEdge({stop.input_id, stop.output_id, {true, Stop->name_, static_cast<double>(settings_.bus_wait_time), 0}});
            stop.input_id += 2;
            stop.output_id += 2;
        }
    }

    void Router::BuildBusesGraph(const TransportCatalogue &catalogue)
    {
        auto &buses = catalogue.GetBuses();
        for (const auto &[busname, Bus] : buses)
        {
            const auto &all_stops = Bus->stops_;
            size_t count_stops = all_stops.size();
            if (Bus->is_roundtrip)
            {
                for (size_t i = 0; i < count_stops - 1; ++i)
                {
                    double dist = 0;
                    int span_count = 0;
                    const domain::Stop *from = all_stops[i];
                    for (size_t j = i + 1; j < count_stops; ++j)
                    {
                        const domain::Stop *to = all_stops[j];
                        dist += static_cast<double>(catalogue.GetDistances(all_stops[j - 1], to));
                        double route_time = dist / (settings_.bus_velocity * TRANSLATE_TO_M_MIN);
                        graph_.AddEdge({stop_id_.at(from->name_).output_id, stop_id_.at(to->name_).input_id, {false, Bus->name_, route_time, ++span_count}});
                    }
                }
            }
            else
            {
                for (size_t i = 0; i < (count_stops - 1) / 2; ++i)
                {
                    double dist = 0;
                    double reverse_dist = 0;
                    int span_count = 0;
                    int reverse_span_count = 0;
                    const domain::Stop *from = all_stops[i];
                    for (size_t j = i + 1; j < (count_stops + 1) / 2; ++j)
                    {
                        const domain::Stop *to = all_stops[j];
                        dist += static_cast<double>(catalogue.GetDistances(all_stops[j - 1], to));
                        reverse_dist += static_cast<double>(catalogue.GetDistances(to, all_stops[j - 1]));
                        double route_time = dist / (settings_.bus_velocity * TRANSLATE_TO_M_MIN);
                        double reverse_route_time = reverse_dist / (settings_.bus_velocity * TRANSLATE_TO_M_MIN);
                        graph_.AddEdge({stop_id_.at(from->name_).output_id, stop_id_.at(to->name_).input_id, {false, Bus->name_, route_time, ++span_count}});
                        graph_.AddEdge({stop_id_.at(to->name_).output_id, stop_id_.at(from->name_).input_id, {false, Bus->name_, reverse_route_time, ++reverse_span_count}});
                    }
                }
            }
        }
    }

    std::optional<std::pair<graph::Router<RouteWeight>::RouteInfo, std::vector<RouteWeight>>> Router::FindRoute(std::string_view from,
                                                                                                                std::string_view to) const
    {
        std::vector<RouteWeight> route_items;
        std::optional<graph::Router<RouteWeight>::RouteInfo> route_info =
            router_->BuildRoute(stop_id_.at(from).input_id, stop_id_.at(to).input_id);
        if (route_info.has_value())
        {
            for (size_t edge_id : route_info.value().edges)
            {
                route_items.emplace_back(graph_.GetEdge(edge_id).weight);
            }
            return std::make_pair(route_info.value(), route_items);
        }
        return std::nullopt;
    }

}
