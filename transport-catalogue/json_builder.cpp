#include "json_builder.h"

using namespace std::literals;

namespace json
{

    DictionaryContext BaseContext::StartDict()
    {
        return DictionaryContext(builder_.StartDict());
    }

    ArrayContext BaseContext::StartArray()
    {
        return ArrayContext(builder_.StartArray());
    }

    BaseContext BaseContext::Value(Node::Value node)
    {
        return builder_.Value(std::move(node));
    }
    KeyContext BaseContext::Key(std::string key)
    {
        return KeyContext(builder_.Key(key));
    }
    BaseContext BaseContext::EndDict()
    {
        return builder_.EndDict();
    }
    BaseContext BaseContext::EndArray()
    {
        return builder_.EndArray();
    }

    Node BaseContext::Build()
    {
        return builder_.Build();
    }

    DictionaryContext KeyContext::Value(Node::Value node)
    {
        return BaseContext::Value(std::move(node));
    }

    ArrayContext ArrayContext::Value(Node::Value node)
    {
        return BaseContext::Value(std::move(node));
    }

    KeyContext Builder::Key(std::string key)
    {
        Node::Value &value = GetCurrentValue();
        if (!std::holds_alternative<Dict>(value))
        {
            throw std::logic_error("Key() outside a dict"s);
        }
        nodes_stack.push_back(&std::get<Dict>(value)[key]);

        return BaseContext(*this);
    }

    BaseContext Builder::Value(Node::Value node)
    {
        AddObject(std::move(node), true);
        return *this;
    }

    DictionaryContext Builder::StartDict()
    {
        AddObject(Dict{}, false);
        return BaseContext(*this);
    }
    BaseContext Builder::EndDict()
    {
        if (!std::holds_alternative<Dict>(GetCurrentValue()))
        {
            throw std::logic_error("object isnt dict"s);
        }
        nodes_stack.pop_back();
        return *this;
    }
    ArrayContext Builder::StartArray()
    {
        AddObject(Array{}, false);
        return BaseContext(*this);
    }
    BaseContext Builder::EndArray()
    {
        if (!std::holds_alternative<Array>(GetCurrentValue()))
        {
            throw std::logic_error("object isnt array"s);
        }
        nodes_stack.pop_back();
        return *this;
    }

    Node Builder::Build()
    {
        if (!nodes_stack.empty())
        {
            throw std::logic_error("invalid json"s);
        }
        return std::move(node_);
    }

    Node::Value &Builder::GetCurrentValue()
    {
        if (nodes_stack.empty())
        {
            throw std::logic_error("invalid json"s);
        }
        return nodes_stack.back()->GetValue();
    }

    const Node::Value &Builder::GetCurrentValue() const
    {
        return const_cast<Builder *>(this)->GetCurrentValue();
    }

    void Builder::AssertNewObjectContext()
    {
        if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue()))
        {
            throw std::logic_error("New object is wrong context"s);
        }
    }

    void Builder::AddObject(Node::Value node, bool flag)
    {
        Node::Value &value = GetCurrentValue();
        if (std::holds_alternative<Array>(value))
        {
            Node &node_arr = std::get<Array>(value).emplace_back(std::move(node));
            if (!flag)
            {
                nodes_stack.push_back(&node_arr);
            }
        }
        else
        {
            AssertNewObjectContext();
            value = std::move(node);
            if (flag)
            {
                nodes_stack.pop_back();
            }
        }
    }
}