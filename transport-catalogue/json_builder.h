#pragma once
#include "json.h"

namespace json
{

    class DictionaryContext;
    class ArrayContext;
    class KeyContext;
    class BaseContext;

    class Builder
    {
    public:
        Builder() : node_(), nodes_stack{&node_} {};
        DictionaryContext StartDict();
        BaseContext EndDict();
        ArrayContext StartArray();
        BaseContext EndArray();
        KeyContext Key(std::string key);
        BaseContext Value(Node::Value value);
        Node Build();

    private:
        Node node_;
        std::vector<Node *> nodes_stack;
        void AddObject(Node::Value node, bool flag);
        const Node::Value &GetCurrentValue() const;
        void AssertNewObjectContext();
        Node::Value &GetCurrentValue();
    };
    class BaseContext
    {
    public:
        BaseContext(Builder &builder) : builder_(builder) {};
        DictionaryContext StartDict();
        BaseContext EndDict();
        ArrayContext StartArray();
        BaseContext EndArray();
        BaseContext Value(Node::Value value);
        KeyContext Key(std::string value);
        Node Build();

    private:
        Builder &builder_;
    };

    class DictionaryContext : public BaseContext
    {
    public:
        DictionaryContext(BaseContext builder) : BaseContext(builder) {};
        DictionaryContext StartDict() = delete;
        ArrayContext StartArray() = delete;
        BaseContext Value(Node node) = delete;
        BaseContext EndArray() = delete;
        Node Build() = delete;
    };
    class KeyContext : public BaseContext
    {
    public:
        KeyContext(BaseContext builder) : BaseContext(builder) {};
        KeyContext Key(std::string key) = delete;
        BaseContext EndArray() = delete;
        BaseContext EndDict() = delete;
        Node Build() = delete;
        DictionaryContext Value(Node::Value value);
    };
    class ArrayContext : public BaseContext
    {
    public:
        ArrayContext(BaseContext builder) : BaseContext(builder) {};
        KeyContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        Node Build() = delete;
        ArrayContext Value(Node::Value value);
    };
}