#include "CircuitValues.h"

SignalValue CircuitValues::get(SignalId signal) const
{
    auto i = _values.find(signal);
    if (i == _values.end())
    {
        return 0;
    }
    assert(i->second != 0);
    return i->second;
}

void CircuitValues::add(SignalId signal, SignalValue value)
{
    if (value == 0)
    {
        return;
    }

    auto i = _values.find(signal);
    if (i != _values.end())
    {
        i->second += value;
        if (i->second == 0)
        {
            _values.erase(i);
        }
    }
    else
    {
        _values.emplace(signal, value);
    }
}

void CircuitValues::add(CircuitValues const & additional)
{
    for (auto const & p : additional)
    {
        add(p.first, p.second);
    }
}
