#include "map_render.h"

using namespace map_render;

SphereProjector MapRender::get_sphere_projector(const std::vector<geo::Coordinates>& points) const {
    return SphereProjector(points.begin(), points.end(), render_settings_.width,
    render_settings_.height, render_settings_.padding);
}

void MapRender::init_sphere_projector(std::vector<geo::Coordinates> points){
    sphere_projector = SphereProjector(points.begin(), points.end(), render_settings_.width,
    render_settings_.height, render_settings_.padding);
}

Render_settings MapRender::get_render_settings() const{
        return render_settings_;
}

int MapRender::get_palette_size() const{
    return render_settings_.color_palette.size();
}

svg::Point SphereProjector::operator()(geo::Coordinates coordinates) const{
            return{
                (coordinates.lng-min_lot_) * zoom_coeff_ + padding_,
                (max_lat_ - coordinates.lat) * zoom_coeff_ + padding_
            };
}

svg::Color MapRender::get_color(int line_number) const {
    return render_settings_.color_palette[line_number];
}

void MapRender::set_line_properties(svg::Polyline& polyline, [[maybe_unused]]int line_number) const{
    using namespace std::literals;
    polyline.SetStrokeColor(get_color(line_number));
    polyline.SetFillColor("none"s);
    polyline.SetStrokeWidth(render_settings_.line_width);
    polyline.SetStrokeLineCap(svg::StrokeLineCup::ROUND);
    polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}
void::MapRender::set_route_text_color_properties(svg::Text& text, const std::string& name,
const svg::Point& position, int palette_number) const{
    text.SetFillColor(get_color(palette_number));
    set_route_common_properties(text, name, position);
}
void MapRender::set_route_text_properties(svg::Text& text, const std::string& name, const svg::Point& position) const{
    text.SetFillColor(render_settings_.underlayer_color);
    text.SetStrokeColor(render_settings_.underlayer_color);
    text.SetStrokeWidth(render_settings_.underlayer_width);
    text.SetStrokeLineCap(svg::StrokeLineCup::ROUND);
    text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    set_route_common_properties(text, name, position);
}

void MapRender::set_route_circle_properties(svg::Circle& circle, const svg::Point& position) const{
    using namespace std::literals;
    circle.SetCenter(position);
    circle.SetRadius(render_settings_.stop_radius);
    circle.SetFillColor("white"s);
}

void MapRender::set_stop_text_properties(svg::Text& text, const std::string& name,
const svg::Point& position) const{
    using namespace std::literals;
    text.SetFillColor(render_settings_.underlayer_color);
    text.SetStrokeColor(render_settings_.underlayer_color);
    text.SetStrokeWidth(render_settings_.underlayer_width);
    text.SetStrokeLineCap(svg::StrokeLineCup::ROUND);
    text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    set_stop_common_properties(text, name, position);
}

void MapRender::set_stop_text_color_properties(svg::Text& text,
const std::string& name, const svg::Point& position) const{
    using namespace std::literals;
    text.SetFillColor("black"s);
    set_stop_common_properties(text, name, position);
}

void MapRender::set_route_common_properties(svg::Text& text, const std::string& name, const svg::Point& position) const{
    using namespace std::literals;
    text.SetPosition(position);
    text.SetOffset(render_settings_.bus_label_offset);
    text.SetFontSize(render_settings_.bus_label_font_size);
    text.SetFontFamily("Verdana"s);
    text.SetFontWeight("bold"s);
    text.SetData(name);
}

void MapRender::set_stop_common_properties(svg::Text& text, const std::string& name, const svg::Point& position) const{
    using namespace std::literals;
    text.SetPosition(position);
    text.SetOffset(render_settings_.stop_label_offset);
    text.SetFontSize(render_settings_.stop_label_font_size);
    text.SetFontFamily("Verdana"s);
    text.SetData(name);
}

 bool SphereProjector::IsZero(double value){
    return std::abs(value) < EPSILON;
 }

 void MapRender::add_line(std::vector<std::pair<domain::Bus*, int>> buses_palette){
    std::vector<geo::Coordinates> stops_coordinates;
    for(auto&[bus, color_number] : buses_palette){
        for(const auto& stop : bus->stops_){
            geo::Coordinates coordinates;
            coordinates.lat = stop->coordinates.lat;
            coordinates.lng = stop->coordinates.lng;
            stops_coordinates.push_back(coordinates);
        }
        svg::Polyline bus_line;
        bool bus_empty = true;
        for(auto& stop : stops_coordinates){
            bus_line.AddPoint(sphere_projector(stop));
            bus_empty = false;
        }
        if(!bus_empty){
            set_line_properties(bus_line, color_number);
            map_document.Add(bus_line);
        }
        stops_coordinates.clear();
    }
}

void MapRender::add_buses_name(std::vector<std::pair<domain::Bus*, int>> buses_palette) {
    svg::Text roundtrip_bus_name;
    svg::Text roundtrip_title_name;
    svg::Text nonroundtrip_bus_name;
    svg::Text nonroundtrip_title_name;
    std::vector<geo::Coordinates> stops_coordinates;
    bool bus_empty = true;
    for(auto&[bus, color_number] : buses_palette){
        for(const auto& stop : bus->stops_){
            geo::Coordinates coordinates;
            coordinates.lat = stop->coordinates.lat;
            coordinates.lng = stop->coordinates.lng;
            stops_coordinates.push_back(coordinates);
            if(bus_empty){
                bus_empty = false;
            }
        }
    if(!bus_empty){
        if(bus->is_roundtrip){
            set_route_text_properties(roundtrip_bus_name, bus->name_,
            sphere_projector(stops_coordinates[0]));
            map_document.Add(roundtrip_bus_name);
            set_route_text_color_properties(roundtrip_title_name, bus->name_,
            sphere_projector(stops_coordinates[0]), color_number);
            map_document.Add(roundtrip_title_name);
        }
        else{
            set_route_text_properties(nonroundtrip_bus_name, bus->name_,
            sphere_projector(stops_coordinates[0]));
            map_document.Add(nonroundtrip_bus_name);
            set_route_text_color_properties(nonroundtrip_title_name, bus->name_,
            sphere_projector(stops_coordinates[0]), color_number);
            map_document.Add(nonroundtrip_title_name);
            if(stops_coordinates[0] != stops_coordinates[stops_coordinates.size()/2]){
                set_route_text_properties(nonroundtrip_bus_name, bus->name_,
                sphere_projector(stops_coordinates[stops_coordinates.size()/2]));
                map_document.Add(nonroundtrip_bus_name);
                set_route_text_color_properties(nonroundtrip_title_name, bus->name_,
                sphere_projector(stops_coordinates[stops_coordinates.size()/2]), color_number);
                map_document.Add(nonroundtrip_title_name);
            }
        }
    }
    bus_empty = false;
    stops_coordinates.clear();
    }
}
void MapRender::add_stops_circle(std::vector<domain::Stop*>& stops) {
    svg::Circle circle;
    for(const auto& stop : stops){
        if(stop){
            set_route_circle_properties(circle, 
            sphere_projector({stop->coordinates.lat, stop->coordinates.lng}));
            map_document.Add(circle);
        }
    }
}

void MapRender::add_stops_name(std::vector<domain::Stop*>& stops) {
    svg::Text stop_name;
    svg::Text stop_name_title;
    for(const auto& stop : stops){
        if(stop){
            set_stop_text_properties(stop_name, stop->name_, 
            sphere_projector({stop->coordinates.lat, stop->coordinates.lng}));
            map_document.Add(stop_name);
            set_stop_text_color_properties(stop_name_title, stop->name_, 
            sphere_projector({stop->coordinates.lat, stop->coordinates.lng}));
            map_document.Add(stop_name_title);
        }
    }
}

void MapRender::GetMapStream(std::ostream& out){
    map_document.Render(out);
}