#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <optional>
#include <vector>
#include <variant>

using namespace std::literals;

namespace svg
{

    struct Rgb
    {
        Rgb() = default;
        Rgb(uint8_t red_color, uint8_t green_color, uint8_t blue_color) : red(red_color),
                                                                          green(green_color), blue(blue_color) {};
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba
    {
        Rgba() = default;
        Rgba(uint8_t red_color, uint8_t green_color, uint8_t blue_color, double alpha_color) : red(red_color),
                                                                                               green(green_color), blue(blue_color), opacity(alpha_color) {};
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{"none"s};
    std::ostream &operator<<(std::ostream &out, Color color);

    struct ColorPrinter
    {
        std::ostream &out;
        void operator()(std::monostate) const
        {
            out << "none"sv;
        }
        void operator()(std::string color) const
        {
            out << color;
        }
        void operator()(Rgb color) const
        {
            out << "rgb("sv << static_cast<int>(color.red) << ',' << static_cast<int>(color.green) << ','
                << static_cast<int>(color.blue) << ")"sv;
        }
        void operator()(Rgba color) const
        {
            out << "rgba("sv << static_cast<int>(color.red) << ',' << static_cast<int>(color.green) << ',' << static_cast<int>(color.blue)
                << ',' << color.opacity << ")"sv;
        }
    };

    struct Point
    {
        Point() = default;
        Point(double x, double y)
            : x(x), y(y)
        {
        }
        double x = 0;
        double y = 0;
    };

    enum class StrokeLineCup
    {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin
    {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream &operator<<(std::ostream &out, const StrokeLineCup object);

    std::ostream &operator<<(std::ostream &out, const StrokeLineJoin object);

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext
    {
        RenderContext(std::ostream &out)
            : out_(out)
        {
        }

        RenderContext(std::ostream &out, int indent_step, int indent = 0)
            : out_(out), indent_step_(indent_step), indent_(indent)
        {
        }

        RenderContext Indented() const
        {
            return {out_, indent_step_, indent_ + indent_step_};
        }

        void RenderIndent() const
        {
            for (int i = 0; i < indent_; ++i)
            {
                out_.put(' ');
            }
        }

        std::ostream &out_;
        int indent_step_ = 0;
        int indent_ = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object
    {
    public:
        void Render(const RenderContext &context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext &context) const = 0;
    };

    template <typename Owner>
    class PathProps
    {
    public:
        Owner &SetFillColor(Color fill_color)
        {
            fill_color_ = std::move(fill_color);
            return AsOwner();
        }
        Owner &SetStrokeColor(Color stroke_color)
        {
            stroke_color_ = std::move(stroke_color);
            return AsOwner();
        }
        Owner &SetStrokeWidth(double width)
        {
            width_ = width;
            return AsOwner();
        }
        Owner &SetStrokeLineCap(StrokeLineCup line_cap)
        {
            line_cup_ = line_cap;
            return AsOwner();
        }
        Owner &SetStrokeLineJoin(StrokeLineJoin line_join)
        {
            line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;
        void RenderAttrs(std::ostream &out) const
        {
            if (fill_color_)
            {
                out << "fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_)
            {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (width_)
            {
                out << " stroke-width=\""sv << *width_ << "\""sv;
            }
            if (line_cup_)
            {
                out << " stroke-linecap=\""sv << *line_cup_ << "\""sv;
            }
            if (line_join_)
            {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        Owner &AsOwner()
        {
            return static_cast<Owner &>(*this);
        }
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCup> line_cup_;
        std::optional<StrokeLineJoin> line_join_;
    };

    class Circle final : public Object, public PathProps<Circle>
    {
    public:
        Circle &SetCenter(Point center);
        Circle &SetRadius(double radius);

    private:
        void RenderObject(const RenderContext &context) const override;
        Point center_;
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline>
    {
    public:
        Polyline &AddPoint(Point point);

    private:
        std::vector<Point> points_;
        void RenderObject(const RenderContext &context) const override;
    };

    class Text final : public Object, public PathProps<Text>
    {
    public:
        Text &SetPosition(Point pos);

        Text &SetOffset(Point offset);

        Text &SetFontSize(uint32_t size);

        Text &SetFontFamily(std::string font_family);

        Text &SetFontWeight(std::string font_weight);

        Text &SetData(std::string data);

    private:
        Point position_ = {0, 0};
        Point offset_ = {0, 0};
        uint32_t font_size_ = 1;
        std::optional<std::string> font_family_;
        std::optional<std::string> font_weight_;
        std::string data_ = "";
        void RenderObject(const RenderContext &context) const override;
    };

    class ObjectContainer;

    class Drawable
    {
    public:
        virtual void Draw(ObjectContainer &container) const = 0;
        virtual ~Drawable() = default;
    };

    class ObjectContainer
    {
    public:
        template <typename obj>
        void Add(obj object)
        {
            AddPtr(std::make_unique<obj>(std::move(object)));
        }
        virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;

    protected:
        ~ObjectContainer() = default;
    };

    class Document : public ObjectContainer
    {
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object> &&obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream &out) const;

        // Прочие методы и данные, необходимые для реализации класса Document
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };
}