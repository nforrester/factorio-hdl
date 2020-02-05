#pragma once

#include "CircuitManager.h"
#include "debug.h"

using WireColor = int_fast8_t;
namespace Wire
{
    WireColor constexpr red = 0;
    WireColor constexpr green = red + 1;
}
WireColor constexpr num_wire_colors = Wire::green + 1;

WireColor other_color(WireColor wire);

class Port
{
public:
    Port(std::string const & log_leader): _log_leader(log_leader)
    {
        debug(1) << _log_leader << "Created Port.\n";
        _wires.fill(invalid_circuit_id);
    }

    void connect(WireColor wire, CircuitId id)
    {
        debug(1) << _log_leader << "Port connected to circuit " << id
                 << " (" << (wire == Wire::green ? "green" : "red") << ")\n";
        CircuitId & w = _wires[wire];
        assert(w == invalid_circuit_id);
        w = id;
    }

    CircuitId circuit_id(WireColor wire) const
    {
        return _wires[wire];
    }

    void disconnect(WireColor wire)
    {
        debug(1) << _log_leader << "Port disconnected from circuit " << _wires[wire]
                 << " (" << (wire == Wire::green ? "green" : "red") << ")\n";
        _wires[wire] = invalid_circuit_id;
    }

    CircuitValues read(CircuitManager const & circuits) const;
    CircuitValues const & read(CircuitManager const & circuits, WireColor color) const;

    void write(CircuitManager & circuits, CircuitValues const & values) const;

private:
    std::array<CircuitId, num_wire_colors> _wires;

    std::string const _log_leader;
};
