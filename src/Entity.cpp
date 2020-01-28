#include "Entity.h"
#include "Factorio.h"

void Entity::_set_port(std::string const & name, Port & port)
{
    /* Only completely disconnected ports are allowed to be exposed without specifying
     * which color wire they should be used with. */
    assert(!port.connected());

    assert(_port_interface_colors.count(&port) == 0);
    _port_interface_colors[&port].insert(Wire::green);
    _port_interface_colors[&port].insert(Wire::red);

    assert(_ports.count(name) == 0);
    _ports[name] = &port;
}

void Entity::_set_port(std::string const & name, Port & port, WireColor interface_color)
{
    _lock(other_color(interface_color), port);

    assert(_port_interface_colors.count(&port) == 0);
    _port_interface_colors[&port].insert(interface_color);

    assert(_ports.count(name) == 0);
    _ports[name] = &port;
}

void Entity::_lock(WireColor color, Port & p)
{
    _factorio.lock(color, p);
}

