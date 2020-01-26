#include "Hysteresis.h"
#include "ConstantCombinator.h"
#include "ArithmeticCombinator.h"
#include "DeciderCombinator.h"

Hysteresis::Hysteresis(Factorio & factorio,
                       SignalId in,
                       SignalId out,
                       SignalValue dead_band_center_point,
                       SignalValue dead_band_half_width,
                       WireColor interface_color):
    Composite(factorio)
{
    auto & c = _new_entity<ConstantCombinator>();
    c.constants.add(in, -dead_band_center_point);

    auto & a = _new_entity<ArithmeticCombinator>(
        in,
        ArithmeticCombinator::Op::DIV,
        dead_band_half_width,
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

    _lock(color, in_port());
    _lock(color, out_port());
}
