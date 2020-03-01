#include "ArithmeticCombinator.h"
#include "src/host/blueprint/Blueprint.h"
#include "src/host/debug.h"

#include <cmath>

ArithmeticCombinator::ArithmeticCombinator(
        Factorio & factorio,
        std::string const & log_leader,
        SignalId lhs,
        Op op,
        SignalId rhs,
        SignalId out):
    Entity(factorio, log_leader),
    _in_port(log_leader + "in > "),
    _out_port(log_leader + "out > "),
    _lhs(lhs),
    _op(op),
    _rhs(rhs),
    _out(out),
    _rhs_const(0)
{
    assert(_rhs < num_signals);
    _common_init();
}

ArithmeticCombinator::ArithmeticCombinator(
        Factorio & factorio,
        std::string const & log_leader,
        SignalId lhs,
        Op op,
        SignalValue rhs_const,
        SignalId out):
    Entity(factorio, log_leader),
    _in_port(log_leader + "in > "),
    _out_port(log_leader + "out > "),
    _lhs(lhs),
    _op(op),
    _rhs(LogicSignal::constant),
    _out(out),
    _rhs_const(rhs_const)
{
    _common_init();
}

void ArithmeticCombinator::_common_init()
{
    assert(_lhs < num_signals || _lhs == LogicSignal::signal_each);
    assert(_out < num_signals || _out == LogicSignal::signal_each);

    if (_lhs != LogicSignal::signal_each)
    {
        assert(_out != LogicSignal::signal_each);
    }

    _set_port("in", _in_port);
    _set_port("out", _out_port);
}

void ArithmeticCombinator::tick(CircuitManager & circuits)
{
    debug(1) << _log_leader << "Start tick\n";
    CircuitValues const in = _in_port.read(circuits);
    CircuitValues out;

    SignalValue rhs;
    if (_rhs == LogicSignal::constant)
    {
        rhs = _rhs_const;
    }
    else
    {
        rhs = in.get(_rhs);
    }

    if (_out == LogicSignal::signal_each)
    {
        assert(_lhs == LogicSignal::signal_each);
        for (auto const & p : in)
        {
            SignalId signal = p.first;
            SignalValue lhs = p.second;
            out.add(signal, _operate(lhs, rhs));
        }
    }
    else
    {
        if (_lhs != LogicSignal::signal_each)
        {
            SignalValue lhs = in.get(_lhs);
            out.add(_out, _operate(lhs, rhs));
        }
        else
        {
            for (auto const & p : in)
            {
                SignalValue lhs = p.second;
                out.add(_out, _operate(lhs, rhs));
            }
        }
    }

    debug(1) << _log_leader << in << " -> " << out << "\n";
    _out_port.write(circuits, out);
    debug(1) << _log_leader << "End tick\n";
}

namespace
{
    SignalValue factorio_pow(SignalValue lhs, SignalValue rhs)
    {
        static double constexpr modulus = pow(2.0, 32.0);

        static_assert(std::same_as<SignalValue, int32_t>); // ensure precision preserved
        return static_cast<SignalValue>(fmod(pow(static_cast<double>(lhs), static_cast<double>(rhs)), modulus));
    }
}

SignalValue ArithmeticCombinator::_operate(SignalValue lhs, SignalValue rhs) const
{
    return operate(_op, lhs, rhs);
}

SignalValue ArithmeticCombinator::operate(
    ArithmeticCombinator::Op op, SignalValue lhs, SignalValue rhs)
{
    switch (op)
    {
    case Op::ADD:
        return static_cast<uint32_t>(lhs) + static_cast<uint32_t>(rhs);
    case Op::SUB:
        return static_cast<uint32_t>(lhs) - static_cast<uint32_t>(rhs);
    case Op::MUL:
        return static_cast<uint32_t>(lhs) * static_cast<uint32_t>(rhs);
    case Op::DIV:
        return rhs == 0 ? 0 : lhs / rhs;
    case Op::MOD:
        return rhs == 0 ? 0 : lhs % rhs;
    case Op::POW:
        return factorio_pow(lhs, rhs);
    case Op::LSH:
        return lhs << rhs;
    case Op::RSH:
        return lhs >> rhs;
    case Op::AND:
        return lhs & rhs;
    case Op::OR:
        return lhs | rhs;
    case Op::XOR:
        return lhs ^ rhs;
    }
    assert(false);
}

int ArithmeticCombinator::to_blueprint_entity(Blueprint::Entity & bpe) const
{
    bpe.name = Signal::arithmetic_combinator;

    bpe.control_behavior.emplace();
    bpe.control_behavior->spec = Blueprint::Entity::ArithmeticConditions();
    auto & ac = std::get<Blueprint::Entity::ArithmeticConditions>(*bpe.control_behavior->spec);

    ac.lhs = _lhs;
    ac.out = _out;
    ac.op = _op;

    if (_rhs == LogicSignal::constant)
    {
        ac.rhs_const = _rhs_const;
    }
    else
    {
        ac.rhs_signal = _rhs;
    }

    return 2;
}
