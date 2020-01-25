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

    SignalValue get(SignalId signal) const
    {
        auto i = _values.find(signal);
        if (i == _values.end())
        {
            return 0;
        }
        assert(i->second != 0);
        return i->second;
    }

    void add(SignalId signal, SignalValue value)
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

    void add(CircuitValues const & additional)
    {
        for (auto const & p : additional)
        {
            add(p.first, p.second);
        }
    }

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
