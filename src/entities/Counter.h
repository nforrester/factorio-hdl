#pragma once

#include "src/Composite.h"
#include "ArithmeticCombinator.h"

class Counter: public Composite
{
public:
    Counter(Factorio & factorio,
            SignalId out,
            SignalValue rate,
            WireColor interface_color):
        Composite(factorio)
    {
        auto & a = _new_entity<ArithmeticCombinator>(
            out,
            ArithmeticCombinator::Op::ADD,
            rate,
            out);

        WireColor color = other_color(interface_color);
        _connect(color, a.in_port, a.out_port);

        _port = &a.out_port;

        _lock(color, port());
    }

    Port & port() { return *_port; }

private:
    Port * _port;
};
