#include "json.h"

using namespace ::std::literals;

json::Node LoadNode(std::istream &input);

std::string LoadLiteral(std::istream &input)
{
    std::string value;
    while (std::isalpha(input.peek()))
    {
        value.push_back(static_cast<char>(input.get()));
    }
    return value;
}

json::Node LoadNull(std::istream &input)
{
    if (auto literal = LoadLiteral(input); literal == "null"s)
    {
        return json::Node(nullptr);
    }
    else
    {
        throw json::ParsingError("unable to parse "s + literal + " as null"s);
    }
}

json::Node LoadBool(std::istream &input)
{
    if (auto literal = LoadLiteral(input); literal == "true"s || literal == "false"s)
    {
        return json::Node(literal == "true"s);
    }
    else
    {
        throw json::ParsingError("unable to parse "s + literal + " as null"s);
    }
}

json::Node LoadArray(std::istream &input)
{
    std::vector<json::Node> array;
    for (char ch; input >> ch && ch != ']';)
    {
        if (ch != ',')
        {
            input.putback(ch);
        }
        array.push_back(LoadNode(input));
    }
    if (!input)
    {
        throw json::ParsingError("unable to parse array"s);
    }
    return json::Node(std::move(array));
}

json::Node LoadString(std::istream &input)
{
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string str;

    while (true)
    {
        if (it == end)
        {
            throw json::ParsingError("unable to parse string");
        }
        const char ch = *it;
        if (ch == '"')
        {
            ++it;
            break;
        }
        else if (ch == '\\')
        {
            ++it;
            if (it == end)
            {
                throw json::ParsingError("unable to parse string");
            }
            const char esc_ch = *(it);
            switch (esc_ch)
            {
            case 'n':
                str.push_back('\n');
                break;
            case 't':
                str.push_back('\t');
                break;
            case 'r':
                str.push_back('\r');
                break;
            case '"':
                str.push_back('"');
                break;
            case '\\':
                str.push_back('\\');
                break;
            default:
                throw json::ParsingError("invalid esc \\"s + esc_ch);
            }
        }
        else if (ch == '\n' || ch == '\r')
        {
            throw json::ParsingError("invalid line end"s);
        }
        else
        {
            str.push_back(ch);
        }
        ++it;
    }
    return json::Node{std::move(str)};
}

json::Node LoadNumber(std::istream &input)
{
    std::string number;
    auto read_char = [&number, &input]
    {
        number += static_cast<char>(input.get());
        if (!input)
        {
            throw json::ParsingError("unable to read number"s);
        }
    };
    auto read_digits = [&input, read_char]
    {
        if (!std::isdigit(input.peek()))
        {
            throw json::ParsingError("A digit is expected"s);
        }
        else
        {
            while (std::isdigit(input.peek()))
            {
                read_char();
            }
        }
    };
    if (input.peek() == '-')
    {
        read_char();
    }
    if (input.peek() == '0')
    {
        read_char();
    }
    else
    {
        read_digits();
    }

    bool is_int = true;
    if (input.peek() == '.')
    {
        read_char();
        read_digits();
        is_int = false;
    }
    if (int ch = input.peek(); ch == 'e' || ch == 'E')
    {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-')
        {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try
    {
        if (is_int)
        {
            try
            {
                return json::Node(std::stoi(number));
            }
            catch (...)
            {
            }
        }
        return json::Node(std::stod(number));
    }
    catch (...)
    {
        throw json::ParsingError("unable to convert "s + number + " to number"s);
    }
}

json::Node LoadDict(std::istream &input)
{
    json::Dict map_;
    std::string key;
    for (char ch; input >> ch && ch != '}';)
    {
        if (ch == '"')
        {
            key = LoadString(input).AsString();
            if (input >> ch && ch == ':')
            {
                if (map_.find(key) != map_.end())
                {
                    throw json::ParsingError("duplicate key '"s + key + "'found");
                }
                map_.emplace(std::move(key), LoadNode(input));
            }
            else
            {
                throw json::ParsingError(": expected. but '"s + ch + "' found"s);
            }
        }
        else if (ch != ',')
        {
            throw json::ParsingError("',' expected. but '"s + ch + "' found"s);
        }
    }

    if (!input)
    {
        throw json::ParsingError("unable to parse dictionary"s);
    }
    else
    {
        return json::Node(std::move(map_));
    }
}

json::Node LoadNode(std::istream &input)
{
    char ch;

    if (!(input >> ch))
    {
        throw json::ParsingError(""s);
    }
    else
    {

        if (ch == 'n')
        {
            input.putback(ch);
            return LoadNull(input);
        }
        else if (ch == '[')
        {
            return LoadArray(input);
        }
        else if (ch == '{')
        {
            return LoadDict(input);
        }
        else if (ch == '"')
        {
            return LoadString(input);
        }
        else if (ch == 't' || ch == 'f')
        {
            input.putback(ch);
            return LoadBool(input);
        }
        else
        {
            input.putback(ch);
            return LoadNumber(input);
        }
    }
}

const json::Node &json::Document::GetRoot() const
{
    return root_;
}

namespace json
{
    Document Load(std::istream &input)
    {
        return Document{LoadNode(input)};
    }
}

struct PrintContext
{
    std::ostream &out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const
    {
        for (int i = 0; i < indent; ++i)
        {
            out.put(' ');
        }
    }

    [[nodiscard]] PrintContext Indented() const
    {
        return {out,
                indent_step,
                indent + indent_step};
    }
};

void PrintNode(const json::Node &node, const PrintContext &context);

void PrintString(const std::string &value, std::ostream &out)
{
    out.put('"');

    for (const char ch : value)
    {
        switch (ch)
        {
        case '\r':
            out << "\\r";
            break;
        case '\n':
            out << "\\n";
            break;
        case '\t':
            out << "\\t";
            break;
        case '"':
            out << "\\\"";
            break;
        case '\\':
            out << "\\\\";
            ;
            break;
        default:
            out.put(ch);
            break;
        }
    }

    out.put('"');
}

void PrintValue(const std::nullptr_t &, const PrintContext &context)
{
    context.out << "null"s;
}

template <typename Value>
void PrintValue(const Value value, const PrintContext &context)
{
    context.out << value;
}

void PrintValue(const std::string &value, const PrintContext &context)
{
    PrintString(value, context.out);
}

void PrintValue(bool value, const PrintContext &context)
{
    context.out << std::boolalpha << value;
}

[[maybe_unused]] void PrintValue(json::Array values, const PrintContext &context)
{
    std::ostream &out = context.out;
    out << "[\n"sv;
    bool first = true;
    auto inner_context = context.Indented();
    for (const auto &element : values)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out << ",\n";
        }
        inner_context.PrintIndent();
        PrintNode(element, inner_context);
    }
    out.put('\n');
    context.PrintIndent();
    out.put(']');
}

[[maybe_unused]] void PrintValue(json::Dict values, const PrintContext &context)
{
    std::ostream &out = context.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_context = context.Indented();
    for (const auto &[key, value] : values)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out << ",\n"sv;
        }
        inner_context.PrintIndent();
        PrintString(key, context.out);
        out << ": "sv;
        PrintNode(value, inner_context);
    }
    out.put('\n');
    context.PrintIndent();
    out.put('}');
}

void PrintNode(const json::Node &node, const PrintContext &out)
{
    std::visit([&out](const auto &value)
               { PrintValue(value, out); }, node.GetValue());
}

namespace json
{
    void Print(const Document &doc, std::ostream &out)
    {
        PrintNode(doc.GetRoot(), PrintContext{out});
    }
}

bool json::Node::IsNull() const
{
    if (std::holds_alternative<std::nullptr_t>(value_))
    {
        return true;
    }
    return false;
}

bool json::Node::IsDouble() const
{
    if (std::holds_alternative<double>(value_))
    {
        return true;
    }
    if (std::holds_alternative<int>(value_))
    {
        return true;
    }
    return false;
}

bool json::Node::IsPureDouble() const
{
    if (std::holds_alternative<double>(value_))
    {
        return true;
    }
    return false;
}

bool json::Node::IsInt() const
{
    if (std::holds_alternative<int>(value_))
    {
        return true;
    }
    return false;
}

bool json::Node::IsBool() const
{
    if (std::holds_alternative<bool>(value_))
    {
        return true;
    }
    return false;
}

bool json::Node::IsString() const
{
    if (std::holds_alternative<std::string>(value_))
    {
        return true;
    }
    return false;
}

bool json::Node::IsArray() const
{
    if (std::holds_alternative<Array>(value_))
    {
        return true;
    }
    return false;
}

bool json::Node::IsMap() const
{
    if (std::holds_alternative<Dict>(value_))
    {
        return true;
    }
    return false;
}

int json::Node::AsInt() const
{
    using namespace std::literals;
    if (IsInt())
    {
        return std::get<int>(value_);
    }
    else
    {
        throw std::logic_error("value is not an int"s);
    }
}

double json::Node::AsDouble() const
{
    using namespace std::literals;
    if (!IsDouble())
    {
        throw std::logic_error("value is not a double"s);
    }
    else if (IsPureDouble())
    {
        return std::get<double>(value_);
    }
    else
    {
        return AsInt();
    }
}

bool json::Node::AsBool() const
{
    using namespace std::literals;
    if (IsBool())
    {
        return std::get<bool>(value_);
    }
    else
    {
        throw std::logic_error("value is not a bool"s);
    }
}

const json::Array &json::Node::AsArray() const
{
    using namespace std::literals;
    if (IsArray())
    {
        return std::get<Array>(value_);
    }
    else
    {
        throw std::logic_error("value is not an array"s);
    }
}

const json::Dict &json::Node::AsMap() const
{
    using namespace std::literals;
    if (IsMap())
    {
        return std::get<Dict>(value_);
    }
    else
    {
        throw std::logic_error("value is not a dictionary"s);
    }
}

const std::string &json::Node::AsString() const
{
    using namespace std::literals;
    if (IsString())
    {
        return std::get<std::string>(value_);
    }
    else
    {
        throw std::logic_error("value is not a string"s);
    }
}