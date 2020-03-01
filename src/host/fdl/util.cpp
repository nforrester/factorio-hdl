#include "util.h"
#include "src/host/util.h"
#include "src/host/blueprint/util.h"

#include <list>

void Fdl::check_valid_top_level_form(S::Exp & form)
{
    S::List * l = form.as_list();
    if (!l || l->l.size() == 0 || !l->l.front()->as_symbol())
    {
        throw S::ParseError(
            form.file,
            form.line,
            "All top level forms must be non-empty lists beginning with symbols.");
    }
}

std::unordered_map<std::string, S::PtrV const *>
Fdl::ast_to_defparts(S::PtrV const & ast)
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
                "Expected: (defpart <symbol> (<list>...) <list>...). Got: " + s->write());
        }

        std::string const & name = defpart.at(1)->as_symbol()->s;
        if (defparts.count(name) > 0)
        {
            S::PtrV const & old_defpart = *defparts.at(name);
            bool same = true;
            if (same)
            {
                same = old_defpart.size() == defpart.size();
            }
            if (same)
            {
                auto old_it = old_defpart.begin();
                auto new_it = defpart.begin();
                while (old_it != old_defpart.end())
                {
                    if ((*old_it)->write() != (*new_it)->write())
                    {
                        same = false;
                        break;
                    }

                    ++old_it;
                    ++new_it;
                }
            }
            if (!same)
            {
                throw S::ParseError(
                    s->file,
                    s->line,
                    "Conflicting duplicate defintions for " + name + ".");
            }
        }
        else
        {
            // TODO check name against list of reserved words
            defparts[name] = &defpart;
        }
    }
    return defparts;
}

std::vector<Fdl::Arg> Fdl::gather_new_part_args(
    S::Exp & body_form,
    std::unordered_map<std::string, SignalId> const & signals,
    std::unordered_map<std::string, SignalValue> const & ints,
    std::unordered_set<std::string> const & outside_wires)
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

                    S::Symbol const & symbol = *sig_val->as_list()->l.at(0)->as_symbol();
                    SignalId sig = signal_from_symbol(symbol, signals);

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
                                "Not an int: " + int_word);
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
                if (!w->as_symbol() || !outside_wires.count(w->as_symbol()->s))
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

            if (outside_wires.count(word))
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

            if (signals.count(word) ||
                word.starts_with("sig:"))
            {
                new_part_args.emplace_back(signal_from_symbol(
                    *s.as_symbol(), signals));
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
            std::optional<ArithmeticCombinator::Op> arith_op = arith_op_from_string(word);
            if (arith_op.has_value()) { new_part_args.emplace_back(*arith_op); }
            if (nargs != new_part_args.size())
            {
                continue;
            }
        }

        throw S::ParseError(s.file, s.line, "Unrecognized argument " + s.write());
    }

    return new_part_args;
}

std::optional<ArithmeticCombinator::Op> Fdl::arith_op_from_string(std::string const & word)
{
    if (word == "+") { return ArithmeticCombinator::Op::ADD; }
    if (word == "-") { return ArithmeticCombinator::Op::SUB; }
    if (word == "*") { return ArithmeticCombinator::Op::MUL; }
    if (word == "/") { return ArithmeticCombinator::Op::DIV; }
    if (word == "%") { return ArithmeticCombinator::Op::MOD; }
    if (word == "**") { return ArithmeticCombinator::Op::POW; }
    if (word == "<<") { return ArithmeticCombinator::Op::LSH; }
    if (word == ">>") { return ArithmeticCombinator::Op::RSH; }
    if (word == "&") { return ArithmeticCombinator::Op::AND; }
    if (word == "|") { return ArithmeticCombinator::Op::OR; }
    if (word == "^") { return ArithmeticCombinator::Op::XOR; }
    std::optional<ArithmeticCombinator::Op> nothing;
    return nothing;
}

SignalId Fdl::signal_from_symbol(
    S::Symbol const & symbol,
    std::unordered_map<std::string, SignalId> const & signals)
{
    if (signals.count(symbol.s))
    {
        return signals.at(symbol.s);
    }
    else if (symbol.s.starts_with("sig:"))
    {
        try
        {
            return get_signal_id_from_lower_case(symbol.s.substr(4));
        }
        catch (std::out_of_range & e)
        {
            throw S::ParseError(
                symbol.file,
                symbol.line,
                "No such signal: " + symbol.s);
        }
    }
    else
    {
        throw S::ParseError(
            symbol.file,
            symbol.line,
            "Not a signal: " + symbol.s);
    }
}

void Fdl::expand_all_loads(S::PtrV & ast)
{
    std::unordered_set<std::string> loaded_files;

    /* Make a list out of the vector because we're going to insert elements and
     * don't want to invalidate iterators when we do so. */
    std::list<S::Ptr> astl;
    for (auto it = ast.begin();
         it != ast.end();
         ++it)
    {
        astl.emplace_back(std::move(*it));
    }

    auto it = astl.begin();
    while (it != astl.end())
    {
        S::List * l = (*it)->as_list();
        auto & ll = l->l;
        if (ll.front()->as_symbol() && ll.front()->as_symbol()->s == "load")
        {
            if (ll.size() != 2 || !ll.at(1)->as_string())
            {
                throw S::ParseError(
                    l->file,
                    l->line,
                    "Expected (load <string:fdl_file>).");
            }

            std::string const & filename = ll.at(1)->as_string()->s;
            it = astl.erase(it);

            if (loaded_files.count(filename) != 0)
            {
                continue;
            }
            loaded_files.insert(filename);
            S::PtrV loaded_forms = S::consume(read_file(filename), filename, 1);
            bool first = true;
            auto insertion_position = it;
            for (auto loaded_form = loaded_forms.begin();
                 loaded_form != loaded_forms.end();
                 ++loaded_form)
            {
                check_valid_top_level_form(**loaded_form);
                insertion_position = astl.insert(insertion_position, std::move(*loaded_form));
                if (first)
                {
                    it = insertion_position;
                }
                first = false;
                ++insertion_position;
            }
        }
        else
        {
            ++it;
        }
    }

    /* Rebuild the vector now that we're done inserting things. */
    ast.clear();
    for (auto it = astl.begin();
         it != astl.end();
         ++it)
    {
        ast.emplace_back(std::move(*it));
    }
}
