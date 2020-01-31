#pragma once

#include "SExp.h"
#include "src/Composite.h"
#include "src/entities/ArithmeticCombinator.h"
#include "src/entities/DeciderCombinator.h"

#include <unordered_map>
#include <variant>
#include <vector>
#include <string>

class Entity;

namespace Fdl
{
    class Entity;
    class InstantiatedPart;
}

class Fdl::InstantiatedPart: public Composite
{
public:
    /* The vector<string> is the list of outside wires.
     * The bool is whether a decider should output the input count (true) or not (false). */
    // TODO Allow more types of arguments to be given to defparts?
    using Arg = std::variant<
        /* Can be arg to defpart?    y/n */
        std::vector<std::string>, /*  y  List of outside wires to connect to a port. */
        SignalId,                 /*  y  Can be passed to arithmetic or decider. */
        SignalValue,              /*  y  Can be passed to arithmetic or decider. */
        CircuitValues,            /*  n  Can be passed to constant. */
        ArithmeticCombinator::Op, /*  n  Can be passed to arithmetic. */
        DeciderCombinator::Op,    /*  n  Can be passed to decider. */
        bool>;                    /*  n  Can be passed to decider (true -> write input counts). */

    InstantiatedPart(
        Factorio & factorio,
        std::string const & part_type,
        std::vector<Arg> const & provided_args,
        std::string const & instantiation_file,
        size_t instantiation_line,
        std::unordered_map<std::string, S::PtrV const *> const & defparts);

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
};
