#pragma once

#include <vector>

#include "CircuitValues.h"
#include "Circuit.h"

using CircuitId = size_t;
CircuitId constexpr invalid_circuit_id = std::numeric_limits<CircuitId>::max();

class CircuitManager
{
public:
    CircuitId make()
    {
        CircuitId id = _circuits.size();
        _circuits.emplace_back();
        _next.emplace_back();

        assert(id != invalid_circuit_id);
        assert(_circuits.size() == _next.size());

        return id;
    }

    CircuitValues const & read(CircuitId id) const
    {
        return _circuits.at(id).state();
    }

    void write(CircuitId id, CircuitValues const & values)
    {
        _next.at(id).add(values);
    }

    void finish_tick()
    {
        _circuits = _next;
        for (Circuit & c : _next)
        {
            c.clear();
        }
    }

    void reset()
    {
        _circuits.clear();
        _next.clear();
    }

private:
    std::vector<Circuit> _circuits;
    std::vector<Circuit> _next;
};
