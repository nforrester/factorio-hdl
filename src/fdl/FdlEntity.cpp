#include "FdlEntity.h"
#include "FdlMacro.h"
#include "SExp.h"
#include "src/util.h"

#include <sstream>
#include <cstdlib>

Fdl::Entity::Entity(Factorio & factorio,
                    std::string const & part_name,
                    std::string const & fdl_filename):
    Composite(factorio)
{
    S::PtrV ast = S::consume(read_file(fdl_filename), fdl_filename, 1);

    for (auto const & s : ast)
    {
        S::List * l = s->as_list();
        if (!l || l->l.size() == 0 || l->l.front()->as_symbol())
        {
            throw S::ParseError(
                s->file,
                s->line,
                "All top level forms must be non-empty lists beginning with symbols.");
        }
    }

    expand_all_macros(ast);

    // TODO instantiate all the parts
    // TODO wire everything up
}
