#pragma once
#include <algorithm>

#include <unordered_map>
#include <algorithm>
#include <map>

#include "svg.h"
#include "geo.h"
#include "domain.h"

namespace map_render
{

    inline const double EPSILON = 1e-6;
    struct RenderSettings
    {
        double width;
        double height;
        double padding;
        double line_width;
        double stop_radius;
        uint32_t bus_label_font_size;
        svg::Point bus_label_offset;
        uint32_t stop_label_font_size;
        svg::Point stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width;
        std::vector<svg::Color> color_palette;
    };

    class SphereProjector
    {
    public:
        SphereProjector() = default;
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height,
                        double padding) : padding_(padding)
        {
            if (points_begin == points_end)
            {
                return;
            }
            const auto [left_it, right_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs)
                                                                 { return lhs.lng < rhs.lng; });
            min_lot_ = left_it->lng;
            const double max_lot = right_it->lng;

            const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs)
                                                                 { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            std::optional<double> width_zoom;
            if (!IsZero(max_lot - min_lot_))
            {
                width_zoom = (max_width - 2 * padding) / (max_lot - min_lot_);
            }

            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat))
            {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom)
            {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom)
            {
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom)
            {
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

    class MapRender
    {
    public:
        MapRender(RenderSettings &render_settings) : render_settings_(render_settings) {};
        svg::Document RenderMap(const std::map<std::string_view, domain::Bus *> &buses) const;

    private:
        const svg::Color &GetColor(size_t line_number) const;
        void SettingsForRoute(svg::Text &bus_roundtrip, svg::Text &bus_roundtrip_title, const std::string &name,
                              svg::Point x, size_t color_index) const;
        void SetLineProperties(svg::Polyline &polyline, size_t palette_index) const;
        void SetRouteTextProperties(svg::Text &text, const std::string &name, const svg::Point &position) const;
        void SetRouteTextColorProperties(svg::Text &text, const std::string &name,
                                         const svg::Point &position, size_t palette_number) const;
        void SetRouteCircleProperties(svg::Circle &circle, const svg::Point &position) const;
        void SetStopTextProperties(svg::Text &text, const std::string &name, const svg::Point &position) const;
        void SetStopTextColorProperties(svg::Text &text, const std::string &name,
                                        const svg::Point &position) const;
        void SetRouteCommonProperties(svg::Text &text, const std::string &name, const svg::Point &position) const;
        void SetStopCommonProperties(svg::Text &text, const std::string &name, const svg::Point &position) const;
        std::vector<svg::Polyline> AddLine(const std::map<std::string_view,
                                                          domain::Bus *> &buses,
                                           SphereProjector &Sphere_Projector) const;
        std::vector<svg::Text> AddBusesName(const std::map<std::string_view, domain::Bus *> &buses,
                                            SphereProjector &Sphere_Projector) const;
        std::vector<svg::Circle> AddStopsCircle(const std::map<std::string_view, domain::Stop *> &stops,
                                                SphereProjector &Sphere_Projector) const;
        std::vector<svg::Text> AddStopsName(const std::map<std::string_view, domain::Stop *> &stops,
                                            SphereProjector &Sphere_Projector) const;
        RenderSettings &render_settings_;
    };
}