#include "FdlEntity.h"
#include "FdlMacro.h"
#include "FdlInstantiatedPart.h"
#include "SExp.h"
#include "src/util.h"
#include "util.h"

#include <sstream>
#include <cstdlib>

Fdl::Entity::Entity(Factorio & factorio,
                    std::string const & log_leader,
                    std::string const & part_type,
                    std::vector<Arg> const & provided_args,
                    std::unordered_map<std::string, std::set<WireColor>> const &
                        colors_of_outside_wires,
                    std::string const & fdl_filename):
    Composite(factorio, log_leader)
{
    S::PtrV ast = S::consume(read_file(fdl_filename), fdl_filename, 1);
    for (auto const & form : ast)
    {
        check_valid_top_level_form(*form);
    }
    expand_all_loads(ast);
    expand_all_macros(ast, fdl_filename);
    std::unordered_map<std::string, S::PtrV const *> defparts = ast_to_defparts(ast);

    _part = &_new_entity<InstantiatedPart>(
        log_leader,
        part_type,
        provided_args,
        __FILE__,
        __LINE__,
        defparts);

    _part->connect_all(colors_of_outside_wires);

    for (size_t port_idx = 0; port_idx < _part->_outside_ports.size(); ++port_idx)
    {
        InstantiatedPart::Port const & port_info = _part->_outside_ports.at(port_idx);
        Port & port = *_part->ports().at(port_info.name);
        std::set<WireColor> interf = _part->port_interface_colors(port);
        if (interf.size() == 2)
        {
            _set_port(port_info.name, port);
        }
        else
        {
            assert(interf.size() == 1);
            _set_port(port_info.name, port, *interf.begin());
        }
        _port_to_metadata[&port] = &port_info;
    }
}
