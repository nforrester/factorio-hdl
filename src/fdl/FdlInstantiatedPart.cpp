#include "FdlInstantiatedPart.h"
#include "FdlEntity.h"
#include "src/entities/ConstantCombinator.h"
#include "src/blueprint/util.h"
#include "src/util.h"

#include <unordered_set>
#include <cassert>

Fdl::InstantiatedPart::InstantiatedPart(
    std::string const & part_type,
    std::vector<Arg> const & provided_args,
    std::string const & instantiation_file,
    size_t instantiation_line,
    std::unordered_map<std::string, S::PtrV const *> const & defparts,
    Entity & fdl_entity)
{
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

        ConstantCombinator * constant = &fdl_entity._new_entity<ConstantCombinator>();
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
            _entity = &fdl_entity._new_entity<ArithmeticCombinator>(
                std::get<SignalId>(provided_args.at(2)),
                std::get<ArithmeticCombinator::Op>(provided_args.at(3)),
                std::get<SignalId>(provided_args.at(4)),
                std::get<SignalId>(provided_args.at(5)));
        }
        else
        {
            _entity = &fdl_entity._new_entity<ArithmeticCombinator>(
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
            _entity = &fdl_entity._new_entity<DeciderCombinator>(
                std::get<SignalId>(provided_args.at(2)),
                std::get<DeciderCombinator::Op>(provided_args.at(3)),
                std::get<SignalId>(provided_args.at(4)),
                std::get<SignalId>(provided_args.at(5)),
                std::get<bool>(provided_args.at(6)));
        }
        else
        {
            _entity = &fdl_entity._new_entity<DeciderCombinator>(
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
        else
        {
            /* Add a new part. */

            /* Gather args for the new part. */
            std::vector<Arg> new_part_args;
            for (auto it = ll.begin() + 1; it != ll.end(); ++it)
            {
                S::Exp & s = **it;

                if (s.as_list())
                {
                    auto & sl = s.as_list()->l;
                    if (!sl.empty() && sl.front()->as_list())
                    {
                        CircuitValues constants;
                        for (auto & sig_val : sl)
                        {
                            if (!sig_val->as_list() ||
                                sig_val->as_list()->l.size() != 2 ||
                                !sig_val->as_list()->l.at(0)->as_symbol() ||
                                !(sig_val->as_list()->l.at(1)->as_symbol() ||
                                  sig_val->as_list()->l.at(1)->as_int()))
                            {
                                throw S::ParseError(
                                    sig_val->file,
                                    sig_val->line,
                                    "Expected (<signal> <int>), got: " + sig_val->write());
                            }

                            std::string const & word = sig_val->as_list()->l.at(0)->as_symbol()->s;
                            SignalId sig;
                            if (signals.count(word))
                            {
                                sig = signals.at(word);
                            }
                            else if (word.starts_with("sig:"))
                            {
                                try
                                {
                                    sig = get_signal_id_from_lower_case(word.substr(4));
                                }
                                catch (std::out_of_range & e)
                                {
                                    throw S::ParseError(
                                        sig_val->file,
                                        sig_val->line,
                                        "No such signal: " + word);
                                }
                            }
                            else
                            {
                                throw S::ParseError(
                                    sig_val->file,
                                    sig_val->line,
                                    "Not a signal: " + word);
                            }

                            SignalValue value;
                            S::Exp & int_exp = *sig_val->as_list()->l.at(1);
                            if (int_exp.as_int())
                            {
                                value = int_exp.as_int()->v;
                            }
                            else
                            {
                                std::string const & int_word = int_exp.as_symbol()->s;
                                if (ints.count(int_word))
                                {
                                    value = ints.at(int_word);
                                }
                                else
                                {
                                    throw S::ParseError(
                                        int_exp.file,
                                        int_exp.line,
                                        "Not an int: " + word);
                                }
                            }

                            constants.add(sig, value);
                        }
                        new_part_args.emplace_back(constants);
                        continue;
                    }

                    std::vector<std::string> wires;
                    for (auto & w : sl)
                    {
                        if (!w->as_symbol() || !_inside_wires.count(w->as_symbol()->s))
                        {
                            throw S::ParseError(
                                w->file,
                                w->line,
                                w->write() + " is not a wire.");
                        }
                        wires.push_back(w->as_symbol()->s);
                    }
                    new_part_args.emplace_back(wires);
                    continue;
                }

                if (s.as_int())
                {
                    SignalValue value = s.as_int()->v;
                    new_part_args.emplace_back(value);
                    continue;
                }

                if (s.as_symbol())
                {
                    std::string word = s.as_symbol()->s;

                    if (_inside_wires.count(word))
                    {
                        std::vector<std::string> wires;
                        wires.push_back(word);
                        new_part_args.emplace_back(wires);
                        continue;
                    }

                    if (ints.count(word))
                    {
                        new_part_args.emplace_back(ints.at(word));
                        continue;
                    }

                    if (signals.count(word))
                    {
                        new_part_args.emplace_back(signals.at(word));
                        continue;
                    }

                    if (word.starts_with("sig:"))
                    {
                        try
                        {
                            new_part_args.emplace_back(get_signal_id_from_lower_case(word.substr(4)));
                        }
                        catch (std::out_of_range & e)
                        {
                            throw S::ParseError(s.file, s.line, "No such signal: " + word);
                        }
                        continue;
                    }

                    if (word == "one")
                    {
                        new_part_args.emplace_back(false);
                        continue;
                    }

                    if (word == "input-count")
                    {
                        new_part_args.emplace_back(true);
                        continue;
                    }

                    size_t nargs = new_part_args.size();
                    if (word == ">") { new_part_args.emplace_back(DeciderCombinator::Op::GT); }
                    if (word == "<") { new_part_args.emplace_back(DeciderCombinator::Op::LT); }
                    if (word == ">=") { new_part_args.emplace_back(DeciderCombinator::Op::GE); }
                    if (word == "<=") { new_part_args.emplace_back(DeciderCombinator::Op::LE); }
                    if (word == "==") { new_part_args.emplace_back(DeciderCombinator::Op::EQ); }
                    if (word == "!=") { new_part_args.emplace_back(DeciderCombinator::Op::NE); }
                    if (word == "+") { new_part_args.emplace_back(ArithmeticCombinator::Op::ADD); }
                    if (word == "-") { new_part_args.emplace_back(ArithmeticCombinator::Op::SUB); }
                    if (word == "*") { new_part_args.emplace_back(ArithmeticCombinator::Op::MUL); }
                    if (word == "/") { new_part_args.emplace_back(ArithmeticCombinator::Op::DIV); }
                    if (word == "%") { new_part_args.emplace_back(ArithmeticCombinator::Op::MOD); }
                    if (word == "**") { new_part_args.emplace_back(ArithmeticCombinator::Op::POW); }
                    if (word == "<<") { new_part_args.emplace_back(ArithmeticCombinator::Op::LSH); }
                    if (word == ">>") { new_part_args.emplace_back(ArithmeticCombinator::Op::RSH); }
                    if (word == "&") { new_part_args.emplace_back(ArithmeticCombinator::Op::AND); }
                    if (word == "|") { new_part_args.emplace_back(ArithmeticCombinator::Op::OR); }
                    if (word == "^") { new_part_args.emplace_back(ArithmeticCombinator::Op::XOR); }
                    if (nargs != new_part_args.size())
                    {
                        continue;
                    }
                }

                throw S::ParseError(s.file, s.line, "Unrecognized argument " + s.write());
            }

            std::unique_ptr<InstantiatedPart> new_part = std::make_unique<InstantiatedPart>(
                type,
                new_part_args,
                l->file,
                l->line,
                defparts,
                fdl_entity);

            /* Attach wires. */
            size_t const part_idx = _parts.size();
            size_t port_idx = 0;
            bool definitely_norm_colors = false;
            bool definitely_flip_colors = false;
            for (auto const & inside_port : new_part->_outside_ports)
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
                        {
                            definitely_flip_colors = true;
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

            _parts.emplace_back(std::move(new_part));
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
