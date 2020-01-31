#include "FdlInstantiatedPart.h"

#include <unordered_set>
#include <cassert>

Fdl::InstantiatedPart::InstantiatedPart(
    S::PtrV const & defpart,
    std::vector<Arg> const & provided_args,
    std::string const & instantiation_file,
    size_t instantiation_line,
    std::unordered_map<std::string, S::PtrV const *> const & defparts,
    Factorio & factorio)
{
    // TODO initialize names_in_use with list of reserved words
    std::unordered_set<std::string> names_in_use;

    std::unordered_map<std::string, SignalId> signals;
    std::unordered_map<std::string, SignalValue> ints;

    S::PtrV const & declared_args = defpart.at(1)->as_list()->l;

    if (provided_args.size() != declared_args.size())
    {
        throw S::ParseError(
            declared_args.front()->file,
            declared_args.front()->line,
            "Wrong number of arguments to " + defpart.front()->as_symbol()->s +
            ". Expected " + std::to_string(declared_args.size()) +
            ", got " + std::to_string(provided_args.size() - 1));
    }

    /* Compare provided arguments against declared arguments,
     * and check that both are syntactically valid. */
    size_t arg_index = 0;
    auto provided_arg_it = provided_args.begin();
    for (S::Ptr const & declared_arg_s : declared_args)
    {
        ++provided_arg_it;
        Arg const & provided_arg = *provided_arg_it;

        S::PtrV const & declared_arg = declared_arg_s->as_list()->l;
        if (declared_arg.size() == 0 || !declared_arg.front()->as_symbol())
        {
            throw S::ParseError(
                declared_arg_s->file,
                declared_arg_s->line,
                "Each argument must being with a symbol.");
        }

        std::string const & arg_type = declared_arg.front()->as_symbol()->s;
        if (arg_type == "in" || arg_type == "out")
        {
            Port port;
            port.dir = arg_type == "in" ? Port::Dir::in : Port::Dir::out;

            if (declared_arg.size() != 3 ||
                !declared_arg.at(1)->as_symbol() ||
                !declared_arg.at(2)->as_symbol())
            {
                throw S::ParseError(
                    declared_arg_s->file,
                    declared_arg_s->line,
                    "Port definition must consist of 3 symbols.");
            }

            std::string const & color_str = declared_arg.at(1)->as_symbol()->s;
            if (color_str == "red")
            {
                port.color = Color::red;
            }
            else if (color_str == "green")
            {
                port.color = Color::green;
            }
            else if (color_str == "yellow")
            {
                port.color = Color::yellow;
            }
            else
            {
                throw S::ParseError(
                    declared_arg.at(1)->file,
                    declared_arg.at(1)->line,
                    "Unrecognized color " + color_str);
            }

            port.name = declared_arg.at(2)->as_symbol()->s;
            if (!names_in_use.emplace(port.name).second)
            {
                throw S::ParseError(
                    declared_arg.at(2)->file,
                    declared_arg.at(2)->line,
                    "Name already in use: " + port.name);
            }

            if (!std::holds_alternative<std::vector<std::string>>(provided_arg))
            {
                throw S::ParseError(
                    instantiation_file,
                    instantiation_line,
                    "Expected wire or list of wires for argument " +
                    std::to_string(arg_index + 1) + " to " +
                    defpart.front()->as_symbol()->s);
            }

            for (auto const & wire_name : std::get<std::vector<std::string>>(provided_arg))
            {
                port.outside_wires.push_back(wire_name);
            }

            Wire wire;
            wire.color = port.color;

            wire.outside_port = _outside_ports.size();

            _outside_ports.push_back(port);

            assert(_inside_wires.count(port.name) == 0);
            _inside_wires[port.name] = wire;
        }
        else if (arg_type == "signal")
        {
            if (declared_arg.size() != 2 || !declared_arg.at(1)->as_symbol())
            {
                throw S::ParseError(
                    declared_arg_s->file,
                    declared_arg_s->line,
                    "Signal argument declaration must consist of exactly two symbols.");
            }

            std::string const & signal_name = declared_arg.at(1)->as_symbol()->s;

            if (!names_in_use.emplace(signal_name).second)
            {
                throw S::ParseError(
                    declared_arg.at(2)->file,
                    declared_arg.at(2)->line,
                    "Name already in use: " + signal_name);
            }
            if (!std::holds_alternative<SignalId>(provided_arg))
            {
                throw S::ParseError(
                    instantiation_file,
                    instantiation_line,
                    "Expected signal for argument " +
                    std::to_string(arg_index + 1) + " to " +
                    defpart.front()->as_symbol()->s);
            }

            signals[signal_name] = std::get<SignalId>(provided_arg);
        }
        else if (arg_type == "int")
        {
            if (declared_arg.size() != 2 || !declared_arg.at(1)->as_symbol())
            {
                throw S::ParseError(
                    declared_arg_s->file,
                    declared_arg_s->line,
                    "Int argument declaration must consist of exactly two symbols.");
            }

            std::string const & int_name = declared_arg.at(1)->as_symbol()->s;

            if (!names_in_use.emplace(int_name).second)
            {
                throw S::ParseError(
                    declared_arg.at(2)->file,
                    declared_arg.at(2)->line,
                    "Name already in use: " + int_name);
            }
            if (!std::holds_alternative<SignalValue>(provided_arg))
            {
                throw S::ParseError(
                    instantiation_file,
                    instantiation_line,
                    "Expected int for argument " +
                    std::to_string(arg_index + 1) + " to " +
                    defpart.front()->as_symbol()->s);
            }

            ints[int_name] = std::get<SignalValue>(provided_arg);
        }
        else
        {
            throw S::ParseError(
                declared_arg.front()->file,
                declared_arg.front()->line,
                "Unrecognized argument type " + arg_type);
        }

        ++arg_index;
    }

    /* Process the part body, checking that it is syntactically valid. */
    for (auto body_form = defpart.begin() + 2;
         body_form != defpart.end();
         ++body_form)
    {
        S::List * l = (*body_form)->as_list();
        if (!l || l->l.size() == 0 || !l->l.front()->as_symbol())
        {
            throw S::ParseError(
                (*body_form)->file,
                (*body_form)->line,
                "Expected non-empty list starting with a symbol in part body.");
        }
        S::PtrV const & ll = l->l;

        std::string const & type = ll.front()->as_symbol()->s;
        if (type == "yellow")
        {
            throw S::ParseError(
                l->file,
                l->line,
                "Yellow is only an acceptable color for wires attached to outside ports.");
        }
        else if (type == "green" || type == "red")
        {
            /* Add a wire. */
            if (ll.size() != 2 || !ll.at(1)->as_symbol())
            {
                throw S::ParseError(
                    l->file,
                    l->line,
                    "Expected a single symbol for the name of the wire.");
            }

            std::string const & wire_name = ll.at(1)->as_symbol()->s;
            if (!names_in_use.emplace(wire_name).second)
            {
                throw S::ParseError(
                    ll.at(1)->file,
                    ll.at(1)->line,
                    "Name already in use: " + wire_name);
            }
            assert(_inside_wires.count(wire_name) == 0);
            Wire wire;
            wire.color = type == "green" ? Color::green : Color::red;
            _inside_wires[wire_name] = wire;
        }
        else
        {
            /* Add a part. */
            if (type == "constant")
            {
                assert(false);
            }
            else if (type == "arithmetic")
            {
                assert(false);
            }
            else
            if (type == "decider")
            {
                assert(false);
            }
            else
            {
                assert(false);
            }
        }
    }
}
