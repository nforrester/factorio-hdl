#include "Entity.h"
#include "Factorio.h"

void Entity::_set_port(std::string const & name, Port * port)
{
    assert(port);
    assert(_ports.count(name) == 0);
    _ports[name] = port;
}

void Entity::_set_port(std::string const & name, Port * port, WireColor color)
{
    _factorio.lock(other_color(color), *port);
    _set_port(name, port);
}
