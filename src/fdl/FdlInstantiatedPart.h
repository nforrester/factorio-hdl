#pragma once

#include "SExp.h"

#include <unordered_map>
#include <variant>
#include <vector>
#include <string>

class Factorio;
class Entity;

namespace Fdl
{
    class InstantiatedPart;
    class Primitive;
    class Composite;
}

class Fdl::InstantiatedPart
{
public:
    /* The vector<string> is the list of outside wires. */
    using Arg = std::variant<std::vector<std::string>, SignalId, SignalValue>;

    InstantiatedPart(
        S::PtrV const & defpart,
        std::vector<Arg> const & provided_args,
        std::string const & instantiation_file,
        size_t instantiation_line,
        std::unordered_map<std::string, S::PtrV const *> const & defparts,
        Factorio & factorio);

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

        std::vector<std::string> outside_wires;

        /* Also the name of the inside wire to which this port is connected. */
        std::string name;
    };

    struct Wire
    {
        Color color;
        std::optional<size_t> outside_port;
        std::vector<size_t> inside_ports;
    };

private:
    std::vector<Port> _outside_ports;

    std::unordered_map<std::string, Wire> _inside_wires;
};

class Fdl::Primitive: public Fdl::InstantiatedPart
{
private:
    Entity * _entity;
};

class Fdl::Composite: public Fdl::InstantiatedPart
{
private:
    std::vector<InstantiatedPart> _parts;
};
