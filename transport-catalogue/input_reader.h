#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace input {

        struct CommandDescription {
            // Определяет, задана ли команда (поле command непустое)
            explicit operator bool() const {
                return !command.empty();
            }

            bool operator!() const {
                return !operator bool();
            }

            std::string command;
            std::string id;
            std::string description;
        };

        struct StopDistances{
            StopDistances(std::pair<std::string_view, double> distance) : distance_(distance){};
            std::pair<std::string_view, double> distance_;
        };


        class InputReader {
        public:
            /**
             * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
             */
            void ParseLine(std::string_view line);

            /**
             * Наполняет данными транспортный справочник, используя команды из commands_
             */
            void ApplyCommands(TransportCatalogue& catalogue) const;
        private:
            std::vector<CommandDescription> commands_;
        };

        void ReadForStream(TransportCatalogue& transport_catalogue, std::istream& input);
    }
}