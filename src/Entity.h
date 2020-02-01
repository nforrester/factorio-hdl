#pragma once

#include "CircuitManager.h"
#include "Port.h"

#include <map>
#include <set>

class Factorio;

namespace Blueprint
{
    struct Entity;
}

class Entity
{
public:
    virtual void tick(CircuitManager & circuits) const = 0;

    Port & port(std::string const & name) const
    {
        return *_ports.at(name);
    }

    std::set<WireColor> const & port_interface_colors(Port const & port) const
    {
        return _port_interface_colors.at(&port);
    }

    std::map<std::string, Port *> const & ports() const
    {
        return _ports;
    }

    std::vector<Entity*> const & constituent_entities() const
    {
        return _constituent_entities;
    }

    // Sets only bpe.name and bpe.control_behavior
    // Returns the area of the entity.
    virtual int to_blueprint_entity(Blueprint::Entity & bpe) const = 0;

protected:
    Entity(Factorio & factorio): _factorio(factorio)
    {
        _constituent_entities.push_back(this);
    }

    std::vector<Entity*> _constituent_entities;

    void _lock(WireColor color, Port & p);

    void _set_port(std::string const & name, Port & port);
    void _set_port(std::string const & name, Port & port, WireColor interface_color);

private:
    Factorio & _factorio;
    std::map<std::string, Port *> _ports;
    std::map<Port const *, std::set<WireColor>> _port_interface_colors;
};
