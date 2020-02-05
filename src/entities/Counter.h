#pragma once

#include "src/Composite.h"
#include "ArithmeticCombinator.h"

class Counter: public Composite
{
public:
    Counter(Factorio & factorio,
            std::string const & log_leader,
            SignalId out,
            SignalValue rate,
            WireColor interface_color):
        Composite(factorio, log_leader)
    {
        auto & a = _new_entity<ArithmeticCombinator>(
            log_leader + "arithmetic > ",
            out,
            ArithmeticCombinator::Op::ADD,
            rate,
            out);

        WireColor color = other_color(interface_color);
        _connect(color, a.port("in"), a.port("out"));

        _lock(color, a.port("out"));

        _set_port("out", a.port("out"), interface_color);
    }
};
