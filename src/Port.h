#pragma once

#include "CircuitManager.h"

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
    }

    void connect(WireColor wire, CircuitId id)
    {
        CircuitId & w = _wires[wire];
        assert(w == invalid_circuit_id);
        w = id;
    }

    bool connected() const
    {
        for (WireColor w = 0; w < num_wire_colors; ++w)
        {
            CircuitId const c = _wires[w];
            if (c != invalid_circuit_id)
            {
                return true;
            }
        }
        return false;
    }

    void disconnect(WireColor wire)
    {
        _wires[wire] = invalid_circuit_id;
    }

    CircuitValues read(CircuitManager const & circuits) const;
    CircuitValues const & read(CircuitManager const & circuits, WireColor color) const;

    void write(CircuitManager & circuits, CircuitValues const & values) const;

private:
    std::array<CircuitId, num_wire_colors> _wires;
};
