#include "FdlEntity.h"
#include "FdlMacro.h"
#include "FdlInstantiatedPart.h"
#include "SExp.h"
#include "src/host/util.h"
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

    debug(1) << log_leader << "Creating entities.\n";
    _part = &_new_entity<InstantiatedPart>(
        log_leader,
        part_type,
        provided_args,
        __FILE__,
        __LINE__,
        defparts);
    debug(1) << log_leader << "Created entities.\n";

    debug(1) << log_leader << "Connecting wires.\n";
    _part->connect_all(colors_of_outside_wires);
    debug(1) << log_leader << "Connected wires.\n";

    for (size_t port_idx = 0; port_idx < _part->_outside_ports.size(); ++port_idx)
    {
        InstantiatedPart::Port const & port_info = _part->_outside_ports.at(port_idx);
        Port & port = *_part->ports().at(port_info.name);
        if (port_info.color == InstantiatedPart::Color::yellow)
        {
            _set_port(port_info.name, port);
        }
        else if (port_info.color == InstantiatedPart::Color::green)
        {
            _set_port(port_info.name, port, Wire::green);
        }
        else
        {
            assert(port_info.color == InstantiatedPart::Color::red);
            _set_port(port_info.name, port, Wire::red);
        }
        _port_to_metadata[&port].push_back(&port_info);
    }
}
