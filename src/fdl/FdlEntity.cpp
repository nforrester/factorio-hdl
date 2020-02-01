#include "FdlEntity.h"
#include "FdlMacro.h"
#include "FdlInstantiatedPart.h"
#include "SExp.h"
#include "src/util.h"
#include "util.h"

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
    for (auto const & form : ast)
    {
        check_valid_top_level_form(*form);
    }
    expand_all_macros(ast);
    std::unordered_map<std::string, S::PtrV const *> defparts = ast_to_defparts(ast);

    _part = &_new_entity<InstantiatedPart>(
        part_type, provided_args, __FILE__, __LINE__, defparts, part_type + " 0 > ");

    _part->connect_all(colors_of_outside_wires);

    size_t port_idx = 0;
    for (auto const & np : _part->ports())
    {
        std::string const & name = np.first;
        Port & port = *np.second;
        std::set<WireColor> interf = _part->port_interface_colors(port);
        if (interf.size() == 2)
        {
            _set_port(name, port);
        }
        else
        {
            assert(interf.size() == 1);
            _set_port(name, port, *interf.begin());
        }
        _port_to_metadata[&port] = &_part->_outside_ports.at(port_idx);
        ++port_idx;
    }
}
