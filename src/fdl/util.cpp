#include "util.h"

void check_valid_top_level_form(S::Exp const & form)
{
    S::List * l = form.as_list();
    if (!l || l->l.size() == 0 || !l->l.front()->as_symbol())
    {
        throw S::ParseError(
            s->file,
            s->line,
            "All top level forms must be non-empty lists beginning with symbols.");
    }
}

std::unordered_map<std::string, S::PtrV const *>
ast_to_defparts(S::PtrV const & ast)
{
    std::unordered_map<std::string, S::PtrV const *> defparts;
    for (auto const & s : ast)
    {
        S::PtrV const & defpart = s->as_list()->l;
        if (defpart.size() < 4 ||
            defpart.at(0)->as_symbol()->s != "defpart" ||
            !defpart.at(1)->as_symbol() ||
            !defpart.at(2)->as_list() ||
            !std::all_of(defpart.at(2)->as_list()->l.begin(),
                         defpart.at(2)->as_list()->l.end(),
                         [](S::Ptr const & arg) -> bool { return arg->as_list(); }) ||
            !std::all_of(defpart.begin() + 3,
                         defpart.end(),
                         [](S::Ptr const & arg) -> bool { return arg->as_list(); }))
        {
            throw S::ParseError(
                s->file,
                s->line,
                "Expected: (defpart <symbol> (<list>...) <list>...)");
        }

        std::string const & name = defpart.at(1)->as_symbol()->s;
        if (defparts.count(name) > 0)
        {
            throw S::ParseError(
                s->file,
                s->line,
                "Duplicate defintion for " + name + ".");
        }
        // TODO check name against list of reserved words
        defparts[name] = &defpart;
    }
    return defparts;
}

std::vector<Arg> gather_new_part_args(S::Exp const & body_form)
{
    S::List * l = body_form.as_list();
    assert(l);

    std::vector<Arg> new_part_args;
    for (auto it = l->l.begin() + 1; it != l->l.end(); ++it)
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

    return new_part_args;
}
