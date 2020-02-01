#pragma once

#include "src/Composite.h"
#include "FdlInstantiatedPart.h"

namespace Fdl
{
    class Entity;
}

class Fdl::Entity: public Composite
{
public:
    Entity(Factorio & factorio,
           std::string const & part_name,
           std::vector<Arg> const & provided_args,
           std::unordered_map<std::string, std::set<WireColor>> const & colors_of_outside_wires,
           std::string const & fdl_filename);

    std::vector<std::string> const & wires_desired_by_port(Port const & port) const
    {
        return _port_to_metadata.at(&port)->outside_wires;
    }

    bool port_is_input(Port const & port) const
    {
        return _port_to_metadata.at(&port)->dir == InstantiatedPart::Port::Dir::in;
    }

private:
    std::unordered_map<Port const *, InstantiatedPart::Port const *> _port_to_metadata;

    InstantiatedPart * _part;

    friend class InstantiatedPart;
};
