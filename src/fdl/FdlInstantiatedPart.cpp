#include "FdlInstantiatedPart.h"
#include "FdlEntity.h"
#include "src/entities/ConstantCombinator.h"
#include "src/util.h"

#include <unordered_set>
#include <cassert>
#include <iostream>
#include <functional>

Fdl::InstantiatedPart::InstantiatedPart(
        Factorio & factorio,
        std::string const & log_leader,
        std::string const & part_type,
        std::vector<Arg> const & provided_args,
        std::string const & instantiation_file,
        size_t instantiation_line,
        std::unordered_map<std::string, S::PtrV const *> const & defparts):
    Composite(factorio, log_leader)
{
    _log_leader = log_leader;
    _part_type = part_type;

    /* If this is a primitive part, make it so. */
    if (part_type == "constant")
    {
        if (provided_args.size() != 2 ||
            !holds_alternative<std::vector<std::string>>(provided_args.at(0)) ||
            !holds_alternative<CircuitValues>(provided_args.at(1)))
        {
            throw S::ParseError(
                instantiation_file,
                instantiation_line,
                "Expected (constant (<wires>) (<filters>))");
        }

        Port out;
        out.dir = Port::Dir::out;
        out.color = Color::yellow;
        out.name = "out";
        for (auto const & wire_name : std::get<std::vector<std::string>>(provided_args.at(0)))
        {
            out.outside_wires.push_back(wire_name);
        }
        _outside_ports.push_back(out);

        ConstantCombinator * constant = &_new_entity<ConstantCombinator>(log_leader);
        constant->constants = std::get<CircuitValues>(provided_args.at(1));
        _entity = constant;

        return;
    }

    if (part_type == "arithmetic")
    {
        if (provided_args.size() != 6 ||
            !holds_alternative<std::vector<std::string>>(provided_args.at(0)) ||
            !holds_alternative<std::vector<std::string>>(provided_args.at(1)) ||
            !holds_alternative<SignalId>(provided_args.at(2)) ||
            !holds_alternative<ArithmeticCombinator::Op>(provided_args.at(3)) ||
            !(holds_alternative<SignalId>(provided_args.at(4)) ||
              holds_alternative<SignalValue>(provided_args.at(4))) ||
            !holds_alternative<SignalId>(provided_args.at(5)))
        {
            throw S::ParseError(
                instantiation_file,
                instantiation_line,
                "Expected (arithmetic <wires> <wires> <signal> <op> <signal|constant> <signal>)");
        }

        Port in;
        in.dir = Port::Dir::in;
        in.color = Color::yellow;
        in.name = "in";
        for (auto const & wire_name : std::get<std::vector<std::string>>(provided_args.at(0)))
        {
            in.outside_wires.push_back(wire_name);
        }
        _outside_ports.push_back(in);

        Port out;
        out.dir = Port::Dir::out;
        out.color = Color::yellow;
        out.name = "out";
        for (auto const & wire_name : std::get<std::vector<std::string>>(provided_args.at(1)))
        {
            out.outside_wires.push_back(wire_name);
        }
        _outside_ports.push_back(out);

        if (std::holds_alternative<SignalId>(provided_args.at(4)))
        {
            _entity = &_new_entity<ArithmeticCombinator>(
                log_leader,
                std::get<SignalId>(provided_args.at(2)),
                std::get<ArithmeticCombinator::Op>(provided_args.at(3)),
                std::get<SignalId>(provided_args.at(4)),
                std::get<SignalId>(provided_args.at(5)));
        }
        else
        {
            _entity = &_new_entity<ArithmeticCombinator>(
                log_leader,
                std::get<SignalId>(provided_args.at(2)),
                std::get<ArithmeticCombinator::Op>(provided_args.at(3)),
                std::get<SignalValue>(provided_args.at(4)),
                std::get<SignalId>(provided_args.at(5)));
        }

        return;
    }

    if (part_type == "decider")
    {
        if (provided_args.size() != 7 ||
            !holds_alternative<std::vector<std::string>>(provided_args.at(0)) ||
            !holds_alternative<std::vector<std::string>>(provided_args.at(1)) ||
            !holds_alternative<SignalId>(provided_args.at(2)) ||
            !holds_alternative<DeciderCombinator::Op>(provided_args.at(3)) ||
            !(holds_alternative<SignalId>(provided_args.at(4)) ||
              holds_alternative<SignalValue>(provided_args.at(4))) ||
            !holds_alternative<SignalId>(provided_args.at(5)) ||
            !holds_alternative<bool>(provided_args.at(6)))
        {
            throw S::ParseError(
                instantiation_file,
                instantiation_line,
                "Expected (decider <wires> <wires> "
                "<signal> <op> <signal|constant> <signal> <one|input-count>)");
        }

        Port in;
        in.dir = Port::Dir::in;
        in.color = Color::yellow;
        in.name = "in";
        for (auto const & wire_name : std::get<std::vector<std::string>>(provided_args.at(0)))
        {
            in.outside_wires.push_back(wire_name);
        }
        _outside_ports.push_back(in);

        Port out;
        out.dir = Port::Dir::out;
        out.color = Color::yellow;
        out.name = "out";
        for (auto const & wire_name : std::get<std::vector<std::string>>(provided_args.at(1)))
        {
            out.outside_wires.push_back(wire_name);
        }
        _outside_ports.push_back(out);

        if (std::holds_alternative<SignalId>(provided_args.at(4)))
        {
            _entity = &_new_entity<DeciderCombinator>(
                log_leader,
                std::get<SignalId>(provided_args.at(2)),
                std::get<DeciderCombinator::Op>(provided_args.at(3)),
                std::get<SignalId>(provided_args.at(4)),
                std::get<SignalId>(provided_args.at(5)),
                std::get<bool>(provided_args.at(6)));
        }
        else
        {
            _entity = &_new_entity<DeciderCombinator>(
                log_leader,
                std::get<SignalId>(provided_args.at(2)),
                std::get<DeciderCombinator::Op>(provided_args.at(3)),
                std::get<SignalValue>(provided_args.at(4)),
                std::get<SignalId>(provided_args.at(5)),
                std::get<bool>(provided_args.at(6)));
        }

        return;
    }

    /* This must be a composite part. */
    if (!defparts.count(part_type))
    {
        throw S::ParseError(
            instantiation_file,
            instantiation_line,
            "No such part defined: " + part_type);
    }
    S::PtrV const & defpart = *defparts.at(part_type);

    // TODO initialize names_in_use with list of reserved words
    std::unordered_set<std::string> names_in_use;

    std::unordered_map<std::string, SignalId> signals;
    signals["everything"] = LogicSignal::everything;
    signals["anything"] = LogicSignal::anything;
    signals["each"] = LogicSignal::each;

    std::unordered_map<std::string, SignalValue> ints;

    assert(defpart.at(2)->as_list());
    S::PtrV const & declared_args = defpart.at(2)->as_list()->l;

    if (provided_args.size() != declared_args.size())
    {
        throw S::ParseError(
            declared_args.front()->file,
            declared_args.front()->line,
            "Wrong number of arguments to " + defpart.at(1)->as_symbol()->s +
            ". Expected " + std::to_string(declared_args.size()) +
            ", got " + std::to_string(provided_args.size() - 1));
    }

    /* Compare provided arguments against declared arguments,
     * and check that both are syntactically valid. */
    size_t arg_index = 0;
    auto provided_arg_it = provided_args.begin();
    for (S::Ptr const & declared_arg_s : declared_args)
    {
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
                    defpart.at(1)->as_symbol()->s);
            }

            for (auto const & wire_name : std::get<std::vector<std::string>>(provided_arg))
            {
                port.outside_wires.push_back(wire_name);
            }

            Wire wire;
            wire.file = declared_arg.at(2)->file;
            wire.line = declared_arg.at(2)->line;

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
                    defpart.at(1)->as_symbol()->s);
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
                    defpart.at(1)->as_symbol()->s);
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
        ++provided_arg_it;
    }

    /* Process the part body, checking that it is syntactically valid. */
    for (auto body_form = defpart.begin() + 3;
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
            wire.file = ll.at(1)->file;
            wire.line = ll.at(1)->line;
            wire.color = type == "green" ? Color::green : Color::red;
            _inside_wires[wire_name] = wire;
        }
        else if (type == "int")
        {
            if (ll.size() != 3 || !ll.at(1)->as_symbol())
            {
                throw S::ParseError(
                    l->file,
                    l->line,
                    "Expected a single symbol for the name of the int, "
                    "and one more expression for the value.");
            }
            std::string const & int_name = ll.at(1)->as_symbol()->s;
            if (names_in_use.count(int_name))
            {
                throw S::ParseError(l->file, l->line, "Name already in use: " + int_name);
            }
            std::function<SignalValue(S::Exp &)> value_from_expression;
            value_from_expression =
                [&ints, &value_from_expression](S::Exp & v) -> SignalValue
                {
                    if (v.as_int())
                    {
                        return v.as_int()->v;
                    }
                    if (v.as_symbol())
                    {
                        std::string const & int_name = v.as_symbol()->s;
                        if (!ints.count(int_name))
                        {
                            throw S::ParseError(v.file, v.line, "Not an int: " + int_name);
                        }
                        return ints.at(int_name);
                    }
                    S::List * l = v.as_list();
                    if (l && l->l.size() == 3)
                    {
                        S::Symbol * s = l->l.front()->as_symbol();
                        if (s)
                        {
                            std::optional<ArithmeticCombinator::Op> arith_op =
                                arith_op_from_string(s->s);
                            if (arith_op.has_value())
                            {
                                return ArithmeticCombinator::operate(
                                    *arith_op,
                                    value_from_expression(*l->l.at(1)),
                                    value_from_expression(*l->l.at(2)));
                            }
                        }
                    }
                    throw S::ParseError(
                        v.file,
                        v.line,
                        "Expression '" + v.write() + "' is not convertible to an int.");
                };

            ints[int_name] = value_from_expression(*ll.at(2));
        }
        else if (type == "signal")
        {
            if (ll.size() != 3 || !ll.at(1)->as_symbol())
            {
                throw S::ParseError(
                    l->file,
                    l->line,
                    "Expected a single symbol for the name of the signal, "
                    "and one more expression for the value.");
            }
            std::string const & signal_name = ll.at(1)->as_symbol()->s;
            if (names_in_use.count(signal_name))
            {
                throw S::ParseError(l->file, l->line, "Name already in use: " + signal_name);
            }
            std::function<SignalValue(S::Exp &)> signal_from_expression;
            signal_from_expression =
                [&signals, &signal_from_expression](S::Exp & v) -> SignalId
                {
                    if (v.as_symbol())
                    {
                        return signal_from_symbol(v.as_symbol()->s, signals);
                    }
                    S::List * l = v.as_list();
                    if (l && l->l.size() >= 2)
                    {
                        S::Symbol * s = l->l.front()->as_symbol();
                        if (s)
                        {
                            if (s->s == "notsigs")
                            {
                                std::set<SignalId> notsigs;
                                for (auto it = l->l.begin() + 1; it != l->l.end(); ++it)
                                {
                                    notsigs.insert(signal_from_expression(**it));
                                }
                                SignalId sig = 0;
                                while (notsigs.count(sig))
                                {
                                    ++sig;
                                }
                                assert(sig < num_signals);
                                return sig;
                            }
                        }
                    }
                    throw S::ParseError(
                        v.file,
                        v.line,
                        "Expression '" + v.write() + "' is not convertible to a signal.");
                };

            signals[signal_name] = signal_from_expression(*ll.at(2));
        }
        else
        {
            std::cerr << _log_leader << "NEW "
                      << type << " " << _parts.size() << "\n";

            std::unordered_set<std::string> wire_names;
            for (auto const & nw : _inside_wires)
            {
                wire_names.insert(nw.first);
            }

            /* Add a new part. */
            InstantiatedPart & new_part = _new_entity<InstantiatedPart>(
                _log_leader + (*body_form)->write() + " > ",
                type,
                gather_new_part_args(**body_form, signals, ints, wire_names),
                l->file,
                l->line,
                defparts);

            /* Attach wires. */
            size_t const part_idx = _parts.size();
            size_t port_idx = 0;
            bool definitely_norm_colors = false;
            bool definitely_flip_colors = false;
            for (auto const & inside_port : new_part._outside_ports)
            {
                /* If two different red wires touch the same port, that's an error.
                 * If two different green wires touch the same port, that's an error.
                 * If a yellow wire and any other wire touch the same port, that's an error. */
                bool touches_red = false;
                bool touches_green = false;
                bool touches_yellow = false;

                for (std::string const & wire_name : inside_port.outside_wires)
                {
                    Wire & wire = _inside_wires.at(wire_name);
                    if (wire.color == Color::red)
                    {
                        if (touches_red || touches_yellow)
                        {
                            throw S::ParseError(
                                l->file,
                                l->line,
                                "Wiring error at port " + inside_port.name);
                        }
                        touches_red = true;

                        if (inside_port.color == Color::red)
                        {
                            definitely_norm_colors = true;
                        }
                        if (inside_port.color == Color::green)
                        {
                            definitely_flip_colors = true;
                        }
                    }
                    else if (wire.color == Color::green)
                    {
                        if (touches_green || touches_yellow)
                        {
                            throw S::ParseError(
                                l->file,
                                l->line,
                                "Wiring error at port " + inside_port.name);
                        }
                        touches_green = true;

                        if (inside_port.color == Color::green)
                        {
                            definitely_norm_colors = true;
                        }
                        if (inside_port.color == Color::red)
                        { definitely_flip_colors = true;
                        }
                    }
                    else
                    {
                        assert(wire.color == Color::yellow);
                        if (touches_red || touches_green || touches_yellow)
                        {
                            throw S::ParseError(
                                l->file,
                                l->line,
                                "Wiring error at port " + inside_port.name);
                        }
                        touches_yellow = true;

                        if (inside_port.color != Color::yellow)
                        {
                            throw S::ParseError(
                                l->file,
                                l->line,
                                "Yellow wire " + wire_name + " is connected to a non-yellow port.");
                        }
                    }

                    wire.inside_ports.emplace_back(part_idx, port_idx);
                }

                ++port_idx;
            }

            if (definitely_norm_colors && definitely_flip_colors)
            {
                throw S::ParseError(
                    l->file,
                    l->line,
                    "Wires connected to part are not of consistent colors.");
            }

            _parts.emplace_back(&new_part);
        }
    }

    /* Yellow wires may be connected externally to a red, a green wire, or both,
     * and become the appropriate color internally. When connected to both a red and
     * green wire, two wires are generated internally.
     *
     * You are allowed to flip the interface colors when you use a part.
     * This will cause all internal signal colors to flip. */
    for (auto const & name_wire : _inside_wires)
    {
        Wire const & wire = name_wire.second;

        if (wire.inside_ports.size() + wire.outside_port.has_value() < 2)
        {
            throw S::ParseError(
                wire.file,
                wire.line,
                "Wire " + name_wire.first + " is unused.");
        }

        if (wire.outside_port.has_value())
        {
            Port const & outside_port = _outside_ports.at(*wire.outside_port);

            /* A wire connected to an externally facing out port may not also be connected to
             * an internal in port. */
            if (outside_port.dir == Port::Dir::out)
            {
                for (auto const & ippp : wire.inside_ports)
                {
                    if (_parts.at(ippp.first)->_outside_ports.at(ippp.second).dir == Port::Dir::in)
                    {
                        throw S::ParseError(
                            wire.file,
                            wire.line,
                            "Wire " + name_wire.first +
                            " is connected both to an external out port and an internal in port.");
                    }
                }
            }

            /* A wire connected to an externally facing in port may not also be connected to
             * an internal out port. */
            if (outside_port.dir == Port::Dir::in)
            {
                for (auto const & ippp : wire.inside_ports)
                {
                    if (_parts.at(ippp.first)->_outside_ports.at(ippp.second).dir == Port::Dir::out)
                    {
                        throw S::ParseError(
                            wire.file,
                            wire.line,
                            "Wire " + name_wire.first +
                            " is connected both to an external in port and an internal out port.");
                    }
                }
            }
        }
    }
}

void Fdl::InstantiatedPart::connect_all(
    std::unordered_map<std::string, std::set<WireColor>> const & colors_of_outside_wires)
{
    if (_entity)
    {
        for (Port const & port : _outside_ports)
        {
            _set_port(port.name, *_entity->ports().at(port.name));
        }
        return;
    }

    bool definitely_norm_colors = false;
    bool definitely_flip_colors = false;
    for (auto const & p : _outside_ports)
    {
        if (p.outside_wires.size() == 2)
        {
            assert(p.color == Color::yellow);
            assert(colors_of_outside_wires.at(p.outside_wires.at(0)) !=
                   colors_of_outside_wires.at(p.outside_wires.at(1)));
            continue;
        }
        assert(p.outside_wires.size() == 1);

        std::set<WireColor> connected_colors;
        for (std::string const & outside_wire_name : p.outside_wires)
        {
            for (WireColor color : colors_of_outside_wires.at(outside_wire_name))
            {
                assert(connected_colors.count(color) == 0);
                connected_colors.insert(color);
            }
        }

        if (connected_colors.size() == 2)
        {
            assert(p.color == Color::yellow);
            assert(*connected_colors.begin() !=
                   *connected_colors.begin() + 1);
            continue;
        }
        assert(connected_colors.size() == 1);
        WireColor connected_color = *connected_colors.begin();

        if (p.color == Color::yellow)
        {
            continue;
        }

        if (connected_color == ::Wire::green)
        {
            if (p.color == Color::green)
            {
                definitely_norm_colors = true;
            }
            if (p.color == Color::red)
            {
                definitely_flip_colors = true;
            }
        }
        else
        {
            assert(connected_color == ::Wire::red);
            if (p.color == Color::red)
            {
                definitely_norm_colors = true;
            }
            if (p.color == Color::green)
            {
                definitely_flip_colors = true;
            }
        }
    }
    assert(!(definitely_norm_colors && definitely_flip_colors));

    std::unordered_map<std::string, std::set<WireColor>> final_wire_colors;
    for (auto const & nw : _inside_wires)
    {
        std::string const & name = nw.first;
        Wire const & wire = nw.second;

        std::set<WireColor> colors;
        if (wire.color == Color::red)
        {
            colors.insert(definitely_flip_colors ? ::Wire::green : ::Wire::red);
        }
        else if (wire.color == Color::green)
        {
            colors.insert(definitely_flip_colors ? ::Wire::red : ::Wire::green);
        }
        else
        {
            assert(wire.color == Color::yellow);
            if (wire.outside_port.has_value())
            {
                for (std::string const & outside_wire_name :
                     _outside_ports.at(*wire.outside_port).outside_wires)
                {
                    for (WireColor color : colors_of_outside_wires.at(outside_wire_name))
                    {
                        assert(colors.count(color) == 0);
                        colors.insert(color);
                    }
                }
            }
            else
            {
                colors.insert(::Wire::green);
            }
        }
        final_wire_colors[name] = colors;
    }

    for (InstantiatedPart * p : _parts)
    {
        p->connect_all(final_wire_colors);
    }

    for (auto const & nw : _inside_wires)
    {
        std::string const & wire_name = nw.first;
        Wire const & wire = nw.second;

        ::Port * first_port = nullptr;
        for (auto const & ippp : wire.inside_ports)
        {
            InstantiatedPart & part = *_parts.at(ippp.first);
            Port const & port = part._outside_ports.at(ippp.second);
            if (!first_port)
            {
                std::cerr << _log_leader << "HUB   "
                          << part._part_type << " " << ippp.first << " " << port.name
                          << "\n";
                first_port = part.ports().at(port.name);
                continue;
            }
            for (WireColor color : final_wire_colors.at(wire_name))
            {
                std::cerr << _log_leader << "SPOKE "
                          << part._part_type << " " << ippp.first << " " << port.name
                          << (color == ::Wire::green ? " (green)" : " (red)") << "\n";
                _connect(color, *first_port, *part.ports().at(port.name));
            }
        }
    }

    for (Port const & port : _outside_ports)
    {
        size_t part_idx, port_idx;
        std::tie(part_idx, port_idx) = _inside_wires.at(port.name).inside_ports.front();
        InstantiatedPart & inside_part = *_parts.at(part_idx);
        std::string const & inside_port_name = inside_part._outside_ports.at(port_idx).name;
        ::Port & inside_port = *inside_part.ports().at(inside_port_name);

        std::cerr << _log_leader << "SET PORT " << port.name << " = "
                  << inside_part._part_type << " " << part_idx << " " << inside_port_name;
        if (port.color == Color::yellow)
        {
            if (final_wire_colors.at(port.name).size() == 2)
            {
                std::cerr << " (yellow)\n";
                _set_port(port.name, inside_port);
            }
            else
            {
                std::cerr << " (" << (*final_wire_colors.at(port.name).begin() ==
                                      ::Wire::green ? "green" : "red") << ")\n";
                _set_port(port.name, inside_port, *final_wire_colors.at(port.name).begin());
            }
        }
        else if (port.color == Color::red)
        {
            if (definitely_flip_colors)
            {
                std::cerr << " (green)\n";
                _set_port(port.name, inside_port, ::Wire::green);
            }
            else
            {
                std::cerr << " (red)\n";
                _set_port(port.name, inside_port, ::Wire::red);
            }
        }
        else
        {
            assert(port.color == Color::green);
            if (definitely_flip_colors)
            {
                std::cerr << " (red)\n";
                _set_port(port.name, inside_port, ::Wire::red);
            }
            else
            {
                std::cerr << " (green)\n";
                _set_port(port.name, inside_port, ::Wire::green);
            }
        }
    }
}
