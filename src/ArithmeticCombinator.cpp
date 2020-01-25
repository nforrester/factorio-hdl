#include "ArithmeticCombinator.h"

#include <cmath>

ArithmeticCombinator::ArithmeticCombinator(SignalId lhs, Op op, SignalId rhs, SignalId out):
    _lhs(lhs), _op(op), _rhs(rhs), _out(out), _rhs_const(0)
{
    assert(_lhs < num_signals || _lhs == LogicSignal::each);
    assert(_rhs < num_signals);
    assert(_out < num_signals || _out == LogicSignal::each);

    if (_lhs != LogicSignal::each)
    {
        assert(_out != LogicSignal::each);
    }
}

ArithmeticCombinator::ArithmeticCombinator(SignalId lhs, Op op, SignalValue rhs_const, SignalId out):
    _lhs(lhs), _op(op), _rhs(LogicSignal::constant), _out(out), _rhs_const(rhs_const)
{
    assert(_lhs < num_signals || _lhs == LogicSignal::each);
    assert(_out < num_signals || _out == LogicSignal::each);

    if (_lhs != LogicSignal::each)
    {
        assert(_out != LogicSignal::each);
    }
}

void ArithmeticCombinator::tick(CircuitManager & circuits) const
{
    CircuitValues const in = in_port.read(circuits);
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

    out_port.write(circuits, out);
}

SignalValue ArithmeticCombinator::_operate(SignalValue lhs, SignalValue rhs) const
{
    switch (_op)
    {
    case Op::ADD:
        return lhs + rhs;
    case Op::SUB:
        return lhs - rhs;
    case Op::MUL:
        return lhs * rhs;
    case Op::DIV:
        return lhs / rhs;
    case Op::MOD:
        return lhs % rhs;
    case Op::POW:
        static_assert(std::same_as<SignalValue, int32_t>); // ensure precision preserved
        return static_cast<SignalValue>(pow(static_cast<double>(lhs), static_cast<double>(rhs)));
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
