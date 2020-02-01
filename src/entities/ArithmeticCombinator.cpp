#include "ArithmeticCombinator.h"
#include "src/blueprint/Blueprint.h"

#include <cmath>

ArithmeticCombinator::ArithmeticCombinator(Factorio & factorio, SignalId lhs, Op op, SignalId rhs, SignalId out):
    Entity(factorio), _lhs(lhs), _op(op), _rhs(rhs), _out(out), _rhs_const(0)
{
    assert(_rhs < num_signals);
    _common_init();
}

ArithmeticCombinator::ArithmeticCombinator(Factorio & factorio, SignalId lhs, Op op, SignalValue rhs_const, SignalId out):
    Entity(factorio), _lhs(lhs), _op(op), _rhs(LogicSignal::constant), _out(out), _rhs_const(rhs_const)
{
    _common_init();
}

void ArithmeticCombinator::_common_init()
{
    assert(_lhs < num_signals || _lhs == LogicSignal::each);
    assert(_out < num_signals || _out == LogicSignal::each);

    if (_lhs != LogicSignal::each)
    {
        assert(_out != LogicSignal::each);
    }

    _set_port("in", _in_port);
    _set_port("out", _out_port);
}

void ArithmeticCombinator::tick(CircuitManager & circuits) const
{
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

    if (_out == LogicSignal::each)
    {
        assert(_lhs == LogicSignal::each);
        for (auto const & p : in)
        {
            SignalId signal = p.first;
            SignalValue lhs = p.second;
            out.add(signal, _operate(lhs, rhs));
        }
    }
    else
    {
        if (_lhs != LogicSignal::each)
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

    _out_port.write(circuits, out);
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
        return lhs + rhs;
    case Op::SUB:
        return lhs - rhs;
    case Op::MUL:
        return lhs * rhs;
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

    bpe.control_behavior = Blueprint::Entity::ArithmeticConditions();
    auto & ac = std::get<Blueprint::Entity::ArithmeticConditions>(*bpe.control_behavior);

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
