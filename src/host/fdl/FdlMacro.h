#pragma once

#include "SExp.h"

#include <unordered_map>

namespace Fdl
{
    struct Macro
    {
        std::string scheme_function;
        std::string scheme_file;

        S::Ptr execute(S::List const & orig_form) const;
    };

    using MacroMap = std::unordered_map<std::string, Macro>;

    void expand_these_macros(MacroMap const & macros, S::Ptr & ast_node);

    void expand_all_macros(S::PtrV & ast, std::string const & filepath);
}
