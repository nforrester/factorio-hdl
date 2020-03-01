#include "CircuitValues.h"
#include "blueprint/util.h"

#include <vector>

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

std::ostream & operator<<(std::ostream & out, CircuitValues const & values)
{
    std::vector<std::pair<SignalId, SignalValue>> sorted;
    sorted.insert(sorted.end(), values.begin(), values.end());
    std::sort(sorted.begin(), sorted.end());

    out << "[";
    bool first = true;
    for (auto const & v : sorted)
    {
        if (!first)
        {
            out << ", ";
        }
        first = false;

        out << get_signal_name_lower_case(v.first) << " = " << v.second;
    }
    out << "]";

    return out;
}
