#include "svg.h"

namespace svg
{

    using namespace std::literals;

    void Object::Render(const RenderContext &context) const
    {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out_ << std::endl;
    }

    Circle &Circle::SetCenter(Point center)
    {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius)
    {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out_;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }
    Polyline &Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }
    void Polyline::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out_;
        out << "<polyline points=\""sv;
        bool begin_end = true;
        for (const auto &point : points_)
        {
            if (!begin_end)
            {
                out << ' ';
            }
            out << point.x << ","sv << point.y;
            begin_end = false;
        }
        out << "\" ";
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text &Text::SetPosition(Point pos)
    {
        position_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size)
    {
        font_size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family)
    {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text &Text::SetData(std::string data)
    {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out_;
        out << "<text ";
        RenderAttrs(out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\""sv;
        if (font_family_)
        {
            out << " font-family=\""sv << *font_family_ << "\"";
        }
        if (font_weight_)
        {
            out << " font-weight=\""sv << *font_weight_ << "\"";
        }
        out << '>';
        for (const auto symbol : data_)
        {
            if (symbol == '<')
            {
                out << "&lt;"sv;
                continue;
            }
            else if (symbol == '>')
            {
                out << "&gt;"sv;
                continue;
            }
            else if (symbol == '"')
            {
                out << "&quot;"sv;
                continue;
            }
            else if (symbol == '&')
            {
                out << "&amp;";
                continue;
            }
            else if (symbol == '`' || symbol == '\'')
            {
                out << "&apos;";
                continue;
            }
            else
            {
                out << symbol;
            }
        }
        out << "</text>"sv;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object> &&ptr)
    {
        objects_.emplace_back(std::move(ptr));
    }

    void Document::Render(std::ostream &out) const
    {
        out << "<?xml version=\"1.0\" encoding =\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        for (const auto &object : objects_)
        {
            object->Render(ctx);
        }

        out << "</svg>"sv;
    }
    std::ostream &operator<<(std::ostream &out, const StrokeLineJoin object)
    {
        switch (object)
        {
        case StrokeLineJoin::ARCS:
            out << "arсs"s;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"s;
            break;
        default:
            out << "unknown"s;
        }
        return out;
    }
    std::ostream &operator<<(std::ostream &out, const StrokeLineCup object)
    {
        switch (object)
        {
        case StrokeLineCup::BUTT:
            out << "butt"s;
            break;
        case StrokeLineCup::ROUND:
            out << "round"s;
            break;
        case StrokeLineCup::SQUARE:
            out << "square"s;
            break;
        default:
            out << "unknown"s;
            break;
        }
        return out;
    }

    std::ostream &operator<<(std::ostream &out, Color color)
    {
        std::visit(ColorPrinter{out}, color);
        return out;
    }
}