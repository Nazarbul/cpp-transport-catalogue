#include "map_render.h"

using namespace map_render;

svg::Point SphereProjector::operator()(geo::Coordinates coordinates) const
{
    return {
        (coordinates.lng - min_lot_) * zoom_coeff_ + padding_,
        (max_lat_ - coordinates.lat) * zoom_coeff_ + padding_};
}

const svg::Color &MapRender::GetColor(size_t line_number) const
{
    return render_settings_.color_palette[line_number];
}

void MapRender::SetLineProperties(svg::Polyline &polyline, size_t palette_index) const
{
    using namespace std::literals;
    polyline.SetStrokeColor(GetColor(palette_index % render_settings_.color_palette.size()));
    polyline.SetFillColor("none"s);
    polyline.SetStrokeWidth(render_settings_.line_width);
    polyline.SetStrokeLineCap(svg::StrokeLineCup::ROUND);
    polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}
void ::MapRender::SetRouteTextColorProperties(svg::Text &text, const std::string &name,
                                              const svg::Point &position, size_t palette_number) const
{
    text.SetFillColor(GetColor(palette_number));
    SetRouteCommonProperties(text, name, position);
}
void MapRender::SetRouteTextProperties(svg::Text &text, const std::string &name, const svg::Point &position) const
{
    text.SetFillColor(render_settings_.underlayer_color);
    text.SetStrokeColor(render_settings_.underlayer_color);
    text.SetStrokeWidth(render_settings_.underlayer_width);
    text.SetStrokeLineCap(svg::StrokeLineCup::ROUND);
    text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    SetRouteCommonProperties(text, name, position);
}

void MapRender::SetRouteCircleProperties(svg::Circle &circle, const svg::Point &position) const
{
    using namespace std::literals;
    circle.SetCenter(position);
    circle.SetRadius(render_settings_.stop_radius);
    circle.SetFillColor("white"s);
}

void MapRender::SetStopTextProperties(svg::Text &text, const std::string &name,
                                      const svg::Point &position) const
{
    using namespace std::literals;
    text.SetFillColor(render_settings_.underlayer_color);
    text.SetStrokeColor(render_settings_.underlayer_color);
    text.SetStrokeWidth(render_settings_.underlayer_width);
    text.SetStrokeLineCap(svg::StrokeLineCup::ROUND);
    text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    SetStopCommonProperties(text, name, position);
}

void MapRender::SetStopTextColorProperties(svg::Text &text,
                                           const std::string &name, const svg::Point &position) const
{
    using namespace std::literals;
    text.SetFillColor("black"s);
    SetStopCommonProperties(text, name, position);
}

void MapRender::SetRouteCommonProperties(svg::Text &text, const std::string &name, const svg::Point &position) const
{
    using namespace std::literals;
    text.SetPosition(position);
    text.SetOffset(render_settings_.bus_label_offset);
    text.SetFontSize(render_settings_.bus_label_font_size);
    text.SetFontFamily("Verdana"s);
    text.SetFontWeight("bold"s);
    text.SetData(name);
}

void MapRender::SetStopCommonProperties(svg::Text &text, const std::string &name, const svg::Point &position) const
{
    using namespace std::literals;
    text.SetPosition(position);
    text.SetOffset(render_settings_.stop_label_offset);
    text.SetFontSize(render_settings_.stop_label_font_size);
    text.SetFontFamily("Verdana"s);
    text.SetData(name);
}

bool SphereProjector::IsZero(double value)
{
    return std::abs(value) < EPSILON;
}

std::vector<svg::Polyline> MapRender::AddLine(const std::map<std::string_view,
                                                             domain::Bus *> &buses,
                                              SphereProjector &Sphere_Projector) const
{
    std::vector<svg::Polyline> results;
    size_t palette_index = 0;
    for (const auto &[busname, bus] : buses)
    {
        if (bus->stops_.empty())
        {
            continue;
        }
        std::vector<domain::Stop *> stops_for_bus(bus->stops_.begin(), bus->stops_.end());
        svg::Polyline bus_line;
        for (const auto stop : stops_for_bus)
        {
            bus_line.AddPoint(Sphere_Projector(stop->coordinates));
        }
        SetLineProperties(bus_line, palette_index);
        results.push_back(std::move(bus_line));
        ++palette_index;
    }
    return results;
}

void MapRender::SettingsForRoute(svg::Text &roundtrip_bus_name, svg::Text &bus_roundtrip_title, const std::string &name,
                                 svg::Point x, size_t color_index) const
{
    SetRouteTextProperties(roundtrip_bus_name, name, x);
    SetRouteTextColorProperties(bus_roundtrip_title, name, x, color_index);
}

std::vector<svg::Text> MapRender::AddBusesName(const std::map<std::string_view, domain::Bus *> &buses,
                                               SphereProjector &Sphere_Projector) const
{
    std::vector<svg::Text> result;
    size_t color_index = 0;
    svg::Text roundtrip_bus_name;
    svg::Text roundtrip_title_name;
    svg::Text nonroundtrip_bus_name;
    svg::Text nonroundtrip_title_name;
    for (const auto &[busname, bus] : buses)
    {
        if (bus->stops_.empty())
        {
            continue;
        }
        std::vector<domain::Stop *> stops_for_bus(bus->stops_.begin(), bus->stops_.end());
        if (bus->is_roundtrip)
        {
            SettingsForRoute(roundtrip_bus_name, roundtrip_title_name, bus->name_,
                             Sphere_Projector(stops_for_bus[0]->coordinates), color_index);
            result.push_back(std::move(roundtrip_bus_name));
            result.push_back(std::move(roundtrip_title_name));
        }
        else
        {
            SettingsForRoute(nonroundtrip_bus_name, nonroundtrip_title_name, bus->name_,
                             Sphere_Projector(stops_for_bus[0]->coordinates), color_index);
            result.push_back(std::move(nonroundtrip_bus_name));
            result.push_back(std::move(nonroundtrip_title_name));
            if (stops_for_bus[0]->coordinates != stops_for_bus[stops_for_bus.size() / 2]->coordinates)
            {
                SettingsForRoute(nonroundtrip_bus_name, nonroundtrip_title_name, bus->name_,
                                 Sphere_Projector(stops_for_bus[stops_for_bus.size() / 2]->coordinates), color_index);
                result.push_back(std::move(nonroundtrip_bus_name));
                result.push_back(std::move(nonroundtrip_title_name));
            }
        }
        ++color_index;
        color_index %= render_settings_.color_palette.size();
    }
    return result;
}

std::vector<svg::Circle> MapRender::AddStopsCircle(const std::map<std::string_view, domain::Stop *> &stops,
                                                   SphereProjector &Sphere_Projector) const
{
    svg::Circle circle;
    std::vector<svg::Circle> result;
    for (const auto &[name, stop] : stops)
    {
        SetRouteCircleProperties(circle, Sphere_Projector(stop->coordinates));
        result.push_back(std::move(circle));
    }
    return result;
}

std::vector<svg::Text> MapRender::AddStopsName(const std::map<std::string_view, domain::Stop *> &stops, SphereProjector &Sphere_Projector) const
{
    std::vector<svg::Text> result;
    svg::Text stop_name;
    svg::Text stop_name_title;
    for (const auto &[name, stop] : stops)
    {
        SetStopTextProperties(stop_name, stop->name_, Sphere_Projector(stop->coordinates));
        result.push_back(std::move(stop_name));
        SetStopTextColorProperties(stop_name_title, stop->name_,
                                   Sphere_Projector(stop->coordinates));
        result.push_back(std::move(stop_name_title));
    }
    return result;
}

svg::Document MapRender::RenderMap(const std::map<std::string_view, domain::Bus *> &buses) const
{
    svg::Document result;
    std::vector<geo::Coordinates> stop_coordinates;
    std::map<std::string_view, domain::Stop *> sort_stops;

    for (const auto &[bus_name, bus] : buses)
    {
        if (bus->stops_.empty())
        {
            continue;
        }
        else
        {
            for (const auto stop : bus->stops_)
            {
                stop_coordinates.push_back(stop->coordinates);
                sort_stops[stop->name_] = stop;
            }
        }
    }

    SphereProjector Sphere_Projector(stop_coordinates.begin(), stop_coordinates.end(),
                                     render_settings_.width, render_settings_.height, render_settings_.padding);

    for (const auto &line : AddLine(buses, Sphere_Projector))
    {
        result.Add(line);
    }
    for (const auto &text : AddBusesName(buses, Sphere_Projector))
    {
        result.Add(text);
    }

    for (const auto &circle : AddStopsCircle(sort_stops, Sphere_Projector))
    {
        result.Add(circle);
    }

    for (const auto &text : AddStopsName(sort_stops, Sphere_Projector))
    {
        result.Add(text);
    }
    return result;
}