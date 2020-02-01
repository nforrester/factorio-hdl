#pragma once

#include "util.h"
#include "src/Composite.h"

class Entity;

namespace Fdl
{
    class Entity;
    class InstantiatedPart;
}

class Fdl::InstantiatedPart: public Composite
{
public:
    InstantiatedPart(
        Factorio & factorio,
        std::string const & part_type,
        std::vector<Arg> const & provided_args,
        std::string const & instantiation_file,
        size_t instantiation_line,
        std::unordered_map<std::string, S::PtrV const *> const & defparts,
        std::string const & log_leader);

    void connect_all(std::unordered_map<std::string, std::set<WireColor>> const & colors_of_outside_wires);

private:
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
        std::vector<std::pair<size_t, size_t>> inside_ports; // part index, port index

        std::string file;
        size_t line;
    };

    std::vector<Port> _outside_ports;

    std::unordered_map<std::string, Wire> _inside_wires;

    ::Entity * _entity = nullptr;

    std::vector<InstantiatedPart*> _parts;

    std::string _log_leader;
    std::string _part_type;

    friend class Fdl::Entity;
};
