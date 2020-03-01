#include "Port.h"

WireColor other_color(WireColor wire)
{
    static_assert(num_wire_colors == 2);
    if (wire == Wire::red)
    {
        return Wire::green;
    }
    return Wire::red;
}

CircuitValues Port::read(CircuitManager const & circuits) const
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

CircuitValues const & Port::read(CircuitManager const & circuits, WireColor color) const
{
    CircuitId const c = _wires[color];
    assert(c != invalid_circuit_id);
    return circuits.read(c);
}

void Port::write(CircuitManager & circuits, CircuitValues const & values) const
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
