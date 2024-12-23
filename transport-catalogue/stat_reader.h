#pragma once

#include <iosfwd>
#include <string>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace output {

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);

        namespace detail {
            void PrintBus(std::set<std::string_view> buses, std::ostream& output);
        }
    }
}