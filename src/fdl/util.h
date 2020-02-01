#pragma once

#include "SExp.h"
#include "src/entities/ArithmeticCombinator.h"
#include "src/entities/DeciderCombinator.h"

#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include <string>

namespace Fdl
{
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

    void check_valid_top_level_form(S::Exp & form);

    /* Do a basic syntax check on the defpart forms and make a map of them. */
    std::unordered_map<std::string, S::PtrV const *>
    ast_to_defparts(S::PtrV const & ast);

    /* Gather args for a new part. */
    std::vector<Arg> gather_new_part_args(
        S::Exp & body_form,
        std::unordered_map<std::string, SignalId> const & signals,
        std::unordered_map<std::string, SignalValue> const & ints,
        std::unordered_set<std::string> const & outside_wires);

    std::optional<ArithmeticCombinator::Op> arith_op_from_string(std::string const & s);

    SignalId signal_from_symbol(
        S::Symbol const & symbol,
        std::unordered_map<std::string, SignalId> const & signals);
}
