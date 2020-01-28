#pragma once

#include "CircuitManager.h"
#include "Port.h"

#include <map>

class Factorio;

class Entity
{
public:
    virtual void tick(CircuitManager & circuits) const = 0;

    Port & port(std::string const & name) const
    {
        return *_ports.at(name);
    }

    std::map<std::string, Port *> const & ports() const
    {
        return _ports;
    }

protected:
    Entity(Factorio & factorio): _factorio(factorio)
    {
    }

    void _set_port(std::string const & name, Port * port);
    void _set_port(std::string const & name, Port * port, WireColor color);

private:
    Factorio & _factorio;
    std::map<std::string, Port *> _ports;
};
