#pragma once

#include "CircuitManager.h"

#include <iostream>

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
    Port()
    {
        _wires.fill(invalid_circuit_id);
        _lockouts.fill(false);
    }

    void connect(WireColor wire, CircuitId id)
    {
        assert(!_lockouts[wire]);
        CircuitId & w = _wires[wire];
        assert(w == invalid_circuit_id);
        w = id;
    }

    void disconnect(WireColor wire)
    {
        assert(!_lockouts[wire]);
        _wires[wire] = invalid_circuit_id;
    }

    void lockout(WireColor wire)
    {
        _lockouts[wire] = true;
    }

    CircuitValues read(CircuitManager const & circuits) const;
    CircuitValues const & read(CircuitManager const & circuits, WireColor color) const;

    void write(CircuitManager & circuits, CircuitValues const & values) const;

private:
    std::array<CircuitId, num_wire_colors> _wires;
    std::array<bool, num_wire_colors> _lockouts;
};
