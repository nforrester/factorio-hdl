#pragma once

#include "Composite.h"

class Hysteresis: public Composite
{
public:
    Hysteresis(Factorio & factorio,
               SignalId in,
               SignalId out,
               SignalValue set_point,
               SignalValue dead_band,
               WireColor interface_color):
        Composite(factorio)
    {
        auto & c = _new_entity<ConstantCombinator>();
        c.constants.add(in, -set_point);

        auto & a = _new_entity<ArithmeticCombinator>(
            in,
            ArithmeticCombinator::Op::DIV,
            SignalValue(dead_band / 2),
            out);

        auto & d = _new_entity<DeciderCombinator>(
            out,
            DeciderCombinator::Op::GT,
            SignalValue(0),
            out,
            false);

        WireColor color = other_color(interface_color);
        _connect(color, c.port, a.in_port);
        _connect(color, a.out_port, d.in_port);
        _connect(color, d.in_port, d.out_port);

        _in_port = &a.in_port;
        _out_port = &d.out_port;
    }

    Port & in_port() { return *_in_port; }
    Port & out_port() { return *_out_port; }

private:
    Port * _in_port;
    Port * _out_port;
};
