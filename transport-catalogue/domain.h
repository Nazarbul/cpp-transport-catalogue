#pragma once

#include <set>
#include <string>
#include <vector>

#include "geo.h"

namespace domain
{

    struct StatRequest
    {
        int id;
        std::string type;
        std::string name;
    };

    struct Bus;
    struct Stop
    {
        std::string name_;
        geo::Coordinates coordinates;
        std::set<std::string> buses;
    };

    struct Bus
    {
        std::string name_;
        std::vector<Stop *> stops_;
        bool is_roundtrip;
    };

    struct BusInfo
    {
        std::string name_;
        bool not_found;
        size_t count_all_stops;
        size_t count_unique_stops;
        double geo_length;
        double route_length;
        double courvature;
    };

    struct StopInfo
    {
        std::string name_;
        std::vector<std::string> buses_;
        bool not_found;
    };

}