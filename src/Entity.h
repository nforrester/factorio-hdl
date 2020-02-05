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

    // Primitives making up this Entity (constant, arithmetic, and decider combinators).
    std::vector<Entity*> const & primitive_constituents() const
    {
        return _primitive_constituents;
    }

    // For Composite entities, these are the direct children, even if they are themselves Composite.
    std::vector<Entity*> const & direct_constituents() const
    {
        return _direct_constituents;
    }

    // Sets only bpe.name and bpe.control_behavior
    // Returns the area of the entity.
    virtual int to_blueprint_entity(Blueprint::Entity & bpe) const = 0;

protected:
    Entity(Factorio & factorio, std::string const & log_leader):
        _log_leader(log_leader),
        _factorio(factorio)
    {
        // This is undone in Composite's constructor.
        _primitive_constituents.push_back(this);
    }

    std::vector<Entity*> _primitive_constituents;
    std::vector<Entity*> _direct_constituents;

    void _lock(WireColor color, Port & p);

    void _set_port(std::string const & name, Port & port);
    void _set_port(std::string const & name, Port & port, WireColor interface_color);

    std::string const _log_leader;

private:
    Factorio & _factorio;
    std::map<std::string, Port *> _ports;
    std::map<Port const *, std::set<WireColor>> _port_interface_colors;
};
