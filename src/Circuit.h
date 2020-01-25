#pragma once

#include "CircuitValues.h"

class Circuit
{
public:

    Circuit()
    {
    }

    Circuit(Circuit const & other)
    {
        *this = other;
    }

    Circuit & operator=(Circuit const & other)
    {
        _state = other._state;
        return *this;
    }

    void clear()
    {
        _state.clear();
    }

    void add(CircuitValues const & values)
    {
        _state.add(values);
    }

    CircuitValues const & state() const
    {
        return _state;
    }

private:
    CircuitValues _state;
};
