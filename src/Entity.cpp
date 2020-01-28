#include "Entity.h"
#include "Factorio.h"

void Entity::_set_port(std::string const & name, Port & port)
{
    assert(_ports.count(name) == 0);
    _ports[name] = &port;
}

void Entity::_set_port(std::string const & name, Port & port, WireColor interface_color)
{
    _lock(other_color(interface_color), port);
    _set_port(name, port);
}

void Entity::_lock(WireColor color, Port & p)
{
    _factorio.lock(color, p);
}

