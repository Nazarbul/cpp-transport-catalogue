#pragma once

#include <iosfwd>
#include <string>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace output {

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);
        void ReadForStreamandWriteToStream(const TransportCatalogue& transport_catalogue, 
            std::istream& input, std::ostream& output);
        namespace detail {
            void PrintBus(const std::set<Bus*>& buses, std::ostream& output);
            void PrintBusInfo(const BusInfo& bus, std::ostream& output);
        }
    }
}