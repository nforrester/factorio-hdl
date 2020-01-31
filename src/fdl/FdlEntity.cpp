#include "FdlEntity.h"
#include "FdlMacro.h"
#include "FdlInstantiatedPart.h"
#include "SExp.h"
#include "src/util.h"

#include <sstream>
#include <cstdlib>

Fdl::Entity::Entity(Factorio & factorio,
                    std::string const & part_type,
                    std::vector<Arg> const & provided_args,
                    std::unordered_map<std::string, std::set<WireColor>> const &
                        colors_of_outside_wires,
                    std::string const & fdl_filename):
    Composite(factorio)
{
    S::PtrV ast = S::consume(read_file(fdl_filename), fdl_filename, 1);

    // TODO make this section common with the one in FdlMacro.cpp
    for (auto const & s : ast)
    {
        S::List * l = s->as_list();
        if (!l || l->l.size() == 0 || !l->l.front()->as_symbol())
        {
            throw S::ParseError(
                s->file,
                s->line,
                "All top level forms must be non-empty lists beginning with symbols.");
        }
    }

    expand_all_macros(ast);

    /* Do a basic syntax check on the defpart forms and make a map of them. */
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

    InstantiatedPart & part = _new_entity<InstantiatedPart>(
        part_type, provided_args, __FILE__, __LINE__, defparts);

    part.connect_all(colors_of_outside_wires);

    for (auto const & np : part.ports())
    {
        std::string const & name = np.first;
        Port & port = *np.second;
        std::set<WireColor> interf = part.port_interface_colors(port);
        if (interf.size() == 2)
        {
            _set_port(name, port);
        }
        else
        {
            assert(interf.size() == 1);
            _set_port(name, port, *interf.begin());
        }
    }
}
