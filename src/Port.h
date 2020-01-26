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

class Port
{
public:
    Port()
    {
        for (WireColor w = 0; w < num_wire_colors; ++w)
        {
            disconnect(w);
        }
    }

    void connect(WireColor wire, CircuitId id)
    {
        CircuitId & w = _wires[wire];
        assert(w == invalid_circuit_id);
        w = id;
    }

    void disconnect(WireColor wire)
    {
        _wires[wire] = invalid_circuit_id;
    }

    CircuitValues read(CircuitManager const & circuits) const
    {
        CircuitValues values;

        for (WireColor w = 0; w < num_wire_colors; ++w)
        {
            CircuitId const c = _wires[w];
            if (c == invalid_circuit_id)
            {
                continue;
            }

            values.add(circuits.read(c));
        }

        return values;
    }

    void write(CircuitManager & circuits, CircuitValues const & values) const
    {
        for (WireColor w = 0; w < num_wire_colors; ++w)
        {
            CircuitId const c = _wires[w];
            if (c == invalid_circuit_id)
            {
                continue;
            }

            circuits.write(c, values);
        }
    }

private:
    std::array<CircuitId, num_wire_colors> _wires;
};
