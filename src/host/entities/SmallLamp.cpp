#include "SmallLamp.h"
#include "src/blueprint/Blueprint.h"
#include "src/debug.h"

SmallLamp::SmallLamp(
        Factorio & factorio,
        std::string const & log_leader,
        SignalId lhs,
        DeciderCombinator::Op op,
        SignalId rhs,
        bool use_colors):
    Entity(factorio, log_leader),
    _in_port(log_leader + "in > "),
    _lhs(lhs),
    _op(op),
    _rhs(rhs),
    _rhs_const(0),
    _use_colors(use_colors)
{
    assert(_rhs < num_signals);
    _common_init();
}

SmallLamp::SmallLamp(
        Factorio & factorio,
        std::string const & log_leader,
        SignalId lhs,
        DeciderCombinator::Op op,
        SignalValue rhs_const,
        bool use_colors):
    Entity(factorio, log_leader),
    _in_port(log_leader + "in > "),
    _lhs(lhs),
    _op(op),
    _rhs(LogicSignal::constant),
    _rhs_const(rhs_const),
    _use_colors(use_colors)
{
    _common_init();
}

void SmallLamp::_common_init()
{
    assert(_lhs < num_signals ||
           _lhs == LogicSignal::signal_each ||
           _lhs == LogicSignal::signal_anything ||
           _lhs == LogicSignal::signal_everything);

    _set_port("in", _in_port);
}

void SmallLamp::tick(CircuitManager & circuits)
{
    debug(1) << _log_leader << "Start tick\n";
    CircuitValues const in = _in_port.read(circuits);

    SignalValue rhs;
    if (_rhs == LogicSignal::constant)
    {
        rhs = _rhs_const;
    }
    else
    {
        rhs = in.get(_rhs);
    }

    bool condition_met;
    if (_lhs < num_signals)
    {
        SignalValue lhs = in.get(_lhs);
        condition_met = DeciderCombinator::operate(lhs, _op, rhs);
    }
    else if (_lhs == LogicSignal::signal_everything)
    {
        condition_met = true;
        for (auto const & p : in)
        {
            SignalValue lhs = p.second;
            if (!DeciderCombinator::operate(lhs, _op, rhs))
            {
                condition_met = false;
                break;
            }
        }
    }
    else if (_lhs == LogicSignal::signal_anything)
    {
        condition_met = false;
        for (auto const & p : in)
        {
            SignalValue lhs = p.second;
            if (DeciderCombinator::operate(lhs, _op, rhs))
            {
                condition_met = true;
                break;
            }
        }
    }
    else
    {
        assert(false);
    }

    if (condition_met)
    {
        if (_use_colors)
        {
            bool found_color = false;
            static_assert(Signal::signal_black - Signal::signal_red == 8);
            for (SignalId color = Signal::signal_red;
                 color <= Signal::signal_black;
                 ++color)
            {
                if (in.get(color))
                {
                    assert(!found_color);
                    found_color = true;
                    _color = color;
                }
            }
            if (!found_color)
            {
                _color = Signal::signal_white;
            }
        }
        else
        {
            _color = Signal::signal_white;
        }
    }
    else
    {
        _color = Signal::signal_black;
    }

    debug(1) << _log_leader << in << " -> " << get_signal_name_lower_case(_color) << "\n";
    debug(1) << _log_leader << "End tick\n";
}

int SmallLamp::to_blueprint_entity(Blueprint::Entity & bpe) const
{
    bpe.name = Signal::small_lamp;

    bpe.control_behavior.emplace();
    bpe.control_behavior->use_colors = _use_colors;
    bpe.control_behavior->spec = Blueprint::Entity::CircuitCondition();
    auto & cc = std::get<Blueprint::Entity::CircuitCondition>(*bpe.control_behavior->spec);

    cc.lhs = _lhs;
    cc.op = _op;

    if (_rhs == LogicSignal::constant)
    {
        cc.rhs_const = _rhs_const;
    }
    else
    {
        cc.rhs_signal = _rhs;
    }

    return 2;
}
