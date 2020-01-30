#pragma once

namespace Fdl
{
    class InstantiatedPart;
    class Primitive;
    class Composite;
}

class Fdl::InstantiatedPart
{
public:
    enum class Color
    {
        red,
        green,
        yellow,
    };

    struct Wire;

    struct Port
    {
        enum class Dir
        {
            in,
            out,
        };

        Dir dir;
        Color color;
        std::vector<size_t> inside_wires;
        std::vector<size_t> outside_wires;
    };

    struct Wire
    {
        Color color;
        std::vector<Port*> ports;
    };

private:
    std::vector<Port> _ports;

    std::vector<Wire> _wires;
};

class Fdl::Primitive: public Fdl::InstantiatedPart
{
private:
    Entity & _entity;
};

class Fdl::Composite: public Fdl::InstantiatedPart
{
private:
    std::vector<InstantiatedPart> _parts;
};
