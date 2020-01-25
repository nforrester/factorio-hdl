#pragma once

#include <unordered_map>
#include <cassert>

#include "signals.h"

class CircuitValues
{
public:
    using Map = std::unordered_map<SignalId, SignalValue>;

    void clear()
    {
        _values.clear();
    }

    SignalValue get(SignalId signal) const;

    void add(SignalId signal, SignalValue value);

    void add(CircuitValues const & additional);

    Map::const_iterator begin() const
    {
        return _values.cbegin();
    }

    Map::const_iterator end() const
    {
        return _values.cend();
    }

private:
    Map _values;
};
