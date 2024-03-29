#include "Hysteresis.h"
#include "ConstantCombinator.h"
#include "ArithmeticCombinator.h"
#include "DeciderCombinator.h"

Hysteresis::Hysteresis(Factorio & factorio,
                       std::string const & log_leader,
                       SignalId in,
                       SignalId out,
                       SignalValue dead_band_center_point,
                       SignalValue dead_band_half_width,
                       WireColor interface_color):
    Composite(factorio, log_leader)
{
    auto & c = _new_entity<ConstantCombinator>(
        log_leader + "constant > ");
    c.constants.add(in, -dead_band_center_point);

    auto & a = _new_entity<ArithmeticCombinator>(
        log_leader + "arithmetic > ",
        in,
        ArithmeticCombinator::Op::DIV,
        dead_band_half_width,
        out);

    auto & d = _new_entity<DeciderCombinator>(
        log_leader + "decider > ",
        out,
        DeciderCombinator::Op::GT,
        SignalValue(0),
        out,
        false);

    WireColor color = other_color(interface_color);
    _connect(color, c.port("out"), a.port("in"));
    _connect(color, a.port("out"), d.port("in"));
    _connect(color, d.port("in"), d.port("out"));

    _lock(color, a.port("in"));
    _lock(color, d.port("out"));

    _set_port("in", a.port("in"), interface_color);
    _set_port("out", d.port("out"), interface_color);
}
