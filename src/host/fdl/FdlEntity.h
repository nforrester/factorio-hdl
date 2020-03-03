#pragma once

#include "src/host/Composite.h"
#include "FdlInstantiatedPart.h"

namespace Fdl
{
    class Entity;
}

class Fdl::Entity: public Composite
{
public:
    Entity(Factorio & factorio,
           std::string const & log_leader,
           std::string const & part_name,
           std::vector<Arg> const & provided_args,
           std::unordered_map<std::string, std::set<WireColor>> const & colors_of_outside_wires,
           std::string const & fdl_filename);

    std::vector<std::string> wires_desired_by_port(Port const & port) const
    {
        std::vector<std::string> result;
        for (InstantiatedPart::Port const * p : _port_to_metadata.at(&port))
        {
            result.insert(result.end(), p->outside_wires.begin(), p->outside_wires.end());
        }
        return result;
    }

    bool port_is_input(Port const & port) const
    {
        bool result = (*_port_to_metadata.at(&port).begin())->dir == InstantiatedPart::Port::Dir::in;
        for (InstantiatedPart::Port const * p : _port_to_metadata.at(&port))
        {
            assert(result == (p->dir == InstantiatedPart::Port::Dir::in));
        }
        return result;
    }

private:
    std::unordered_map<Port const *, std::vector<InstantiatedPart::Port const *>> _port_to_metadata;

    InstantiatedPart * _part;

    friend class InstantiatedPart;
};
