#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>

using namespace std::literals;


/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
namespace transport_catalogue::detail {
    geo::Coordinates ParseCoordinates(std::string_view str) {
        static const double nan = std::nan("");

        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');

        if (comma == str.npos) {
            return { nan, nan };
        }

        auto not_space2 = str.find_first_not_of(' ', comma + 1);

        double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        double lng = std::stod(std::string(str.substr(not_space2)));
        return { lat, lng };
    }

    /**
     * Удаляет пробелы в начале и конце строки
     */
    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    /**
     * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
     */
    std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;

        size_t pos = 0;
        while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == string.npos) {
                delim_pos = string.size();
            }
            if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }

        return result;
    }

    /**
     * Парсит маршрут.
     * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
     * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
     */
    std::vector<std::string_view> ParseRoute(std::string_view route) {
        if (route.find('>') != route.npos) {
            return Split(route, '>');
        }

        auto stops = Split(route, '-');
        std::vector<std::string_view> results(stops.begin(), stops.end());
        results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

        return results;
    }
    std::pair <std::string_view, std::string_view> ParseStopDistances (std::string_view line){
        line = Trim(line);
        auto symbol_pos = line.find(',');
        if (symbol_pos == line.npos){
            return {};
        }
        auto symbol_pos1 = line.find(',', symbol_pos+1);

        return {line.substr(0, symbol_pos1), line.substr(symbol_pos1+1)};
    }


    std::vector <std::pair <std::string_view, double>> ParseDistances(std::string_view distance){
        distance = Trim(distance);
        std::vector<std::pair<std::string_view, double>> result;
        std::vector<std::string_view> stops = Split(distance, ',');
        for (auto& element : stops){
            auto start = element.find_first_not_of(' ');
            auto delim = element.find('m');
            auto delim1 = element.find('o', delim+1);
            if (delim1==std::string_view::npos || delim == std::string_view::npos 
            || start == std::string_view::npos){
                return result;
            }

            result.push_back({Trim(element.substr(delim1+1)), (std::stod(std::string (element.substr(start, delim - start))))});
        }

        return result;
    }
}
transport_catalogue::input::CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }
    
            return { std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void transport_catalogue::input::InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}


void transport_catalogue::input::InputReader::ApplyCommands
([[maybe_unused]] TransportCatalogue& catalogue) const {
    // Add all stops
    for (CommandDescription cmd : commands_) {
        if (cmd.command == "Stop"s) {
            std::pair <std::string_view, std::string_view> description = transport_catalogue::
            detail::ParseStopDistances(cmd.description);
            Stop stop = { cmd.id, transport_catalogue::detail::ParseCoordinates(description.first)};
            catalogue.AddStop(stop);
        }
    }

    for (CommandDescription cmd : commands_){
        if(cmd.command == "Stop"s){
            std::pair<std::string_view, std::string_view> description = transport_catalogue::detail::ParseStopDistances(cmd.description);
            for (const auto& element : transport_catalogue::detail::ParseDistances(description.second)){
                catalogue.SetDistances(cmd.id, element.first, element.second);
            }
        }
    }

    // Add all buses
    for (CommandDescription cmd : commands_) {
        if (cmd.command == "Bus"s) {
            std::vector<std::string_view> route = transport_catalogue::detail::ParseRoute(cmd.description);
            std::vector<const Stop*>stops;
            for (const auto element : route) {
                stops.push_back(catalogue.FindStop(element));
            }
            Bus bus = { cmd.id, stops };
            catalogue.AddBus(bus);
        }
    }   
}

void transport_catalogue::input::ReadForStream(TransportCatalogue& transport_catalogue, std::istream& input){
        int base_request_count;
        input >> base_request_count >> std::ws;
        {
            transport_catalogue::input::InputReader reader;
            for (int i = 0; i < base_request_count; ++i) {
                std::string line;
                getline(input, line);
                reader.ParseLine(line);
            }
        reader.ApplyCommands(transport_catalogue);
        }
}