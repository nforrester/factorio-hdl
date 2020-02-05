#pragma once

#include "CircuitValues.h"

#include <vector>
#include <iostream> // TODO remove

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
        std::cout << _log_leader << id << "? " << _circuits.at(id) << "\n";
        return _circuits.at(id);
    }

    void write(CircuitId id, CircuitValues const & values)
    {
        std::cout << _log_leader << id << ": " << _next.at(id) << " + " << values << " = ";
        _next.at(id).add(values);
        std::cout << _next.at(id) << "\n";
    }

    bool stable()
    {
        return _circuits == _next;
    }

    void finish_tick()
    {
        std::cout << _log_leader << "Finish Tick.\n";
        _circuits = _next;
        for (CircuitValues & c : _next)
        {
            c.clear();
        }

        int i = 0;
        for (auto const & c : _circuits)
        {
            std::cout << _log_leader << "Values on " << i++ << ": " << c << "\n";
        }
    }

    void reset()
    {
        _circuits.clear();
        _next.clear();
    }

private:
    std::vector<CircuitValues> _circuits;
    std::vector<CircuitValues> _next;
    std::string const _log_leader = "CircuitManager > ";
};
