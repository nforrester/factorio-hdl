#pragma once

#include "Entity.h"

class DeciderCombinator: public Entity
{
public:
    enum class Op
    {
        GT,
        LT,
        GE,
        LE,
        EQ,
        NE
    };

    static_assert(!std::same_as<SignalId, SignalValue>); // permit overload of constructor on these types

    DeciderCombinator(SignalId lhs, Op op, SignalId rhs, SignalId out, bool write_input_count):
        _lhs(lhs),
        _op(op),
        _rhs(rhs),
        _out(out),
        _rhs_const(0),
        _write_input_count(write_input_count)
    {
        assert(_lhs < num_signals ||
               _lhs == LogicSignal::each ||
               _lhs == LogicSignal::anything ||
               _lhs == LogicSignal::everything);
        assert(_rhs < num_signals);
        assert(_out < num_signals ||
               _out == LogicSignal::each ||
               _out == LogicSignal::everything);

        if (_lhs != LogicSignal::each)
        {
            assert(_out != LogicSignal::each);
        }
        else
        {
            assert(_out != LogicSignal::everything);
        }
    }

    DeciderCombinator(SignalId lhs, Op op, SignalValue rhs_const, SignalId out, bool write_input_count):
        _lhs(lhs),
        _op(op),
        _rhs(LogicSignal::constant),
        _out(out),
        _rhs_const(rhs_const),
        _write_input_count(write_input_count)
    {
        assert(_lhs < num_signals ||
               _lhs == LogicSignal::each ||
               _lhs == LogicSignal::anything ||
               _lhs == LogicSignal::everything);
        assert(_out < num_signals ||
               _out == LogicSignal::each ||
               _out == LogicSignal::everything);

        if (_lhs != LogicSignal::each)
        {
            assert(_out != LogicSignal::each);
        }
        else
        {
            assert(_out != LogicSignal::everything);
        }
    }

    void tick(CircuitManager & circuits) const override
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
                if (_operate(lhs, rhs))
                {
                    out.add(signal, _write_input_count ? in.get(signal) : 1);
                }
            }
        }
        else
        {
            if (_lhs != LogicSignal::each)
            {
                bool condition_met;
                if (_lhs < num_signals)
                {
                    SignalValue lhs = in.get(_lhs);
                    condition_met = _operate(lhs, rhs);
                }
                else if (_lhs == LogicSignal::everything)
                {
                    condition_met = true;
                    for (auto const & p : in)
                    {
                        SignalValue lhs = p.second;
                        if (!_operate(lhs, rhs))
                        {
                            condition_met = false;
                            break;
                        }
                    }
                }
                else if (_lhs == LogicSignal::anything)
                {
                    condition_met = false;
                    for (auto const & p : in)
                    {
                        SignalValue lhs = p.second;
                        if (_operate(lhs, rhs))
                        {
                            condition_met = true;
                            break;
                        }
                    }
                }

                if (_out == LogicSignal::everything)
                {
                    if (condition_met)
                    {
                        for (auto const & p : in)
                        {
                            SignalId signal = p.first;
                            SignalValue in_value = p.second;
                            out.add(signal, _write_input_count ? in_value : 1);
                        }
                    }
                }
                else
                {
                    assert(_out < num_signals);
                    if (condition_met)
                    {
                        out.add(_out, _write_input_count ? in.get(_out) : 1);
                    }
                }
            }
            else
            {
                SignalValue output_value = 0;
                for (auto const & p : in)
                {
                    SignalValue lhs = p.second;
                    if (_operate(lhs, rhs))
                    {
                        output_value += _write_input_count ? lhs : 1;
                    }
                }
                out.add(_out, output_value);
            }
        }

        out_port.write(circuits, out);
    }

    Port in_port;
    Port out_port;

private:
    SignalId const _lhs;
    Op const _op;
    SignalId const _rhs;
    SignalId const _out;

    SignalValue _rhs_const;

    bool _write_input_count;

    bool _operate(SignalValue lhs, SignalValue rhs) const
    {
        switch (_op)
        {
        case Op::GT:
            return lhs > rhs;
        case Op::LT:
            return lhs < rhs;
        case Op::GE:
            return lhs >= rhs;
        case Op::LE:
            return lhs <= rhs;
        case Op::EQ:
            return lhs == rhs;
        case Op::NE:
            return lhs != rhs;
        }
        assert(false);
    }
};
