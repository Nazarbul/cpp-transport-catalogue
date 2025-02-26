#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <iostream>
#include <sstream>

using namespace std::literals;

namespace json
{

    class Node;
    using Array = std::vector<Node>;
    using Dict = std::map<std::string, Node>;

    class ParsingError : public std::runtime_error
    {
        using runtime_error::runtime_error;
    };

    class Node
    {
    public:
        using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;
        Node() = default;
        Node(Array array) : value_(std::move(array)) {};
        Node(Dict map) : value_(std::move(map)) {};
        Node(int value) : value_(value) {};
        Node(std::string value) : value_(std::move(value)) {};
        Node(double value) : value_(value) {};
        Node(bool value) : value_(value) {};
        Node(std::nullptr_t) : Node() {};
        bool IsNull() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsMap() const;
        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string &AsString() const;
        const Array &AsArray() const;
        const Dict &AsMap() const;
        const Value &GetValue() const
        {
            return value_;
        }

    private:
        Value value_;
    };

    inline bool operator==(const Node &left, const Node &right)
    {
        return left.GetValue() == right.GetValue();
    }
    inline bool operator!=(const Node &left, const Node &right)
    {
        return !(left == right);
    }

    class Document
    {
    public:
        Document() = default;
        explicit Document(Node node) : root_(std::move(node)) {};
        const Node &GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream &input);

    inline bool operator==(const Document &left, const Document &right)
    {
        return left.GetRoot() == right.GetRoot();
    }
    inline bool operator!=(const Document &left, const Document &right)
    {
        return !(left == right);
    }

    void Print(const Document &doc, std::ostream &out);
}