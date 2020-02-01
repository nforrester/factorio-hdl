#include "DeciderCombinator.h"
#include "src/blueprint/Blueprint.h"

DeciderCombinator::DeciderCombinator(Factorio & factorio, SignalId lhs, Op op, SignalId rhs, SignalId out, bool write_input_count):
    Entity(factorio),
    _lhs(lhs),
    _op(op),
    _rhs(rhs),
    _out(out),
    _rhs_const(0),
    _write_input_count(write_input_count)
{
    assert(_rhs < num_signals);
    _common_init();
}

DeciderCombinator::DeciderCombinator(Factorio & factorio, SignalId lhs, Op op, SignalValue rhs_const, SignalId out, bool write_input_count):
    Entity(factorio),
    _lhs(lhs),
    _op(op),
    _rhs(LogicSignal::constant),
    _out(out),
    _rhs_const(rhs_const),
    _write_input_count(write_input_count)
{
    _common_init();
}

void DeciderCombinator::_common_init()
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

    _set_port("in", _in_port);
    _set_port("out", _out_port);
}

void DeciderCombinator::tick(CircuitManager & circuits) const
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

    _out_port.write(circuits, out);
}

bool DeciderCombinator::_operate(SignalValue lhs, SignalValue rhs) const
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

int DeciderCombinator::to_blueprint_entity(Blueprint::Entity & bpe) const
{
    bpe.name = Signal::decider_combinator;

    bpe.control_behavior = Blueprint::Entity::DeciderConditions();
    auto & dc = std::get<Blueprint::Entity::DeciderConditions>(*bpe.control_behavior);

    dc.lhs = _lhs;
    dc.out = _out;
    dc.op = _op;
    dc.copy_count_from_input = _write_input_count;

    if (_rhs == LogicSignal::constant)
    {
        dc.rhs_const = _rhs_const;
    }
    else
    {
        dc.rhs_signal = _rhs;
    }

    return 2;
}
