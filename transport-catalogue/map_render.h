#pragma once
#include<algorithm>

#include "svg.h"
#include "geo.h"
#include "domain.h"


namespace map_render{

inline const double EPSILON = 1e-6;
    struct Render_settings{
        double width;
        double height;
        double padding;
        double line_width;
        double stop_radius;
        double bus_label_font_size;
        svg::Point bus_label_offset;
        double stop_label_font_size;
        svg::Point stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width;
        std::vector<svg::Color> color_palette;
    };

    class SphereProjector{
        public:
        SphereProjector() = default;
        template<typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, 
        double padding) : padding_(padding){
            if(points_begin == points_end){
                return;
            }
        const auto [left_it, right_it] = std::minmax_element(points_begin, points_end, [](auto lhs,
        auto rhs){
            return lhs.lng < rhs.lng;
        });
        min_lot_ = left_it->lng;
        const double max_lot = right_it->lng;

        const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end, [](auto lhs,
        auto rhs){
            return lhs.lat < rhs.lat;
        });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if(!IsZero(max_lot - min_lot_)){
            width_zoom = (max_width - 2 * padding)/(max_lot - min_lot_);
        }

        std::optional<double> height_zoom;
        if(!IsZero(max_lat_ - min_lat)){
            height_zoom = (max_height - 2 * padding)/(max_lat_ - min_lat);
        }

        if(width_zoom && height_zoom){
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if(width_zoom){
            zoom_coeff_ = *width_zoom;
        }
        else if(height_zoom){
            zoom_coeff_ = *height_zoom;
        }
        }

        svg::Point operator()(geo::Coordinates coordinates) const;

        private:
            double padding_ = 0.0;
            double min_lot_ = 0.0;
            double max_lat_ = 0.0;
            double zoom_coeff_ = 0.0;
            bool IsZero(double value);

        };

        class MapRender{
            public:
            MapRender(Render_settings& render_settings) : render_settings_(render_settings){};
            Render_settings get_render_settings() const;
            int get_palette_size () const;
            svg::Color get_color(int line_number) const;
            void set_line_properties(svg::Polyline& polyline, int line_number) const;
            void set_route_text_properties(svg::Text& text, const std::string& name, const svg::Point& position) const;
            void set_route_text_color_properties(svg::Text& text, const std::string& name,
            const svg::Point& position, int palette_number) const;
            void set_route_circle_properties(svg::Circle& circle, const svg::Point& position) const;
            void set_stop_text_properties(svg::Text& text, const std::string& name, const svg::Point& position) const;
            void set_stop_text_color_properties(svg::Text& text, const std::string& name,
            const svg::Point& position) const;
            SphereProjector get_sphere_projector(const std::vector<geo::Coordinates>& points) const;
            void init_sphere_projector(std::vector<geo::Coordinates> points);
            void add_line(std::vector<std::pair<domain::Bus*, int>> buses_palette);
            void add_buses_name(std::vector<std::pair<domain::Bus*, int>> buses_palette);
            void add_stops_circle(std::vector<domain::Stop*>& stops);
            void add_stops_name(std::vector<domain::Stop*>& stops);
            void GetMapStream(std::ostream& out);
            private:
            void set_route_common_properties(svg::Text& text, const std::string& name, const svg::Point& position) const;
            void set_stop_common_properties(svg::Text& text, const std::string& name, const svg::Point& position) const;
            Render_settings& render_settings_;
            svg::Document map_document;
            SphereProjector sphere_projector;
        };
}