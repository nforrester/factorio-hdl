#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include <cmath>

// TODO make rollover defined behavior
using SignalValue = int32_t;

using SignalId = uint_fast16_t;
SignalId constexpr num_signals = 60 + 37 + 60 + 56 + 48;

namespace LogicSignal
{
    SignalId constexpr max_id = std::numeric_limits<SignalId>::max();

    SignalId constexpr everything = max_id - 1;
    SignalId constexpr anything = everything - 1;
    SignalId constexpr each = anything - 1;
    SignalId constexpr constant = each - 1;

    SignalId constexpr min_id = constant;
}

static_assert(num_signals <= LogicSignal::min_id);

class CircuitValues
{
public:
    using Cont = std::unordered_map<SignalId, SignalValue>;

    void clear()
    {
        _values.clear();
    }

    SignalValue get(SignalId signal) const
    {
        auto i = _values.find(signal);
        if (i == _values.end())
        {
            return 0;
        }
        assert(i->second != 0);
        return i->second;
    }

    void add(SignalId signal, SignalValue value)
    {
        if (value == 0)
        {
            return;
        }

        auto i = _values.find(signal);
        if (i != _values.end())
        {
            i->second += value;
            if (i->second == 0)
            {
                _values.erase(i);
            }
        }
        else
        {
            _values.emplace(signal, value);
        }
    }

    void add(CircuitValues const & additional)
    {
        for (auto const & p : additional)
        {
            add(p.first, p.second);
        }
    }

    Cont::const_iterator begin() const
    {
        return _values.cbegin();
    }

    Cont::const_iterator end() const
    {
        return _values.cend();
    }

private:
    Cont _values;
};

class Circuit
{
public:

    Circuit()
    {
    }

    Circuit(Circuit const & other)
    {
        *this = other;
    }

    Circuit & operator=(Circuit const & other)
    {
        _state = other._state;
        return *this;
    }

    void clear()
    {
        _state.clear();
    }

    void add(CircuitValues const & values)
    {
        _state.add(values);
    }

    CircuitValues const & state() const
    {
        return _state;
    }

private:
    CircuitValues _state;
};

using CircuitId = size_t;
CircuitId constexpr invalid_circuit_id = std::numeric_limits<CircuitId>::max();

class CircuitManager
{
public:
    CircuitId make()
    {
        CircuitId id = _circuits.size();
        _circuits.emplace_back();
        _next.emplace_back();

        assert(id != invalid_circuit_id);
        assert(_circuits.size() == _next.size());

        return id;
    }

    CircuitValues const & read(CircuitId id) const
    {
        return _circuits.at(id).state();
    }

    void write(CircuitId id, CircuitValues const & values)
    {
        _next.at(id).add(values);
    }

    void finish_tick()
    {
        _circuits = _next;
        for (Circuit & c : _next)
        {
            c.clear();
        }
    }

    void reset()
    {
        _circuits.clear();
        _next.clear();
    }

private:
    std::vector<Circuit> _circuits;
    std::vector<Circuit> _next;
};

using WireColor = size_t;
WireColor constexpr num_wire_colors = 2;

class Port
{
public:
    Port()
    {
        for (WireColor w = 0; w < num_wire_colors; ++w)
        {
            disconnect(w);
        }
    }

    void connect(WireColor wire, CircuitId id)
    {
        CircuitId & w = _wires[wire];
        assert(w == invalid_circuit_id);
        w = id;
    }

    void disconnect(WireColor wire)
    {
        _wires[wire] = invalid_circuit_id;
    }

    CircuitValues read(CircuitManager const & circuits) const
    {
        CircuitValues values;

        for (WireColor w = 0; w < num_wire_colors; ++w)
        {
            CircuitId const c = _wires[w];
            if (c == invalid_circuit_id)
            {
                continue;
            }

            values.add(circuits.read(c));
        }

        return values;
    }

    void write(CircuitManager & circuits, CircuitValues const & values) const
    {
        for (WireColor w = 0; w < num_wire_colors; ++w)
        {
            CircuitId const c = _wires[w];
            if (c == invalid_circuit_id)
            {
                continue;
            }

            circuits.write(c, values);
        }
    }

private:
    std::array<CircuitId, num_wire_colors> _wires;
};

class Entity
{
public:
    virtual void tick(CircuitManager & circuits) const = 0;
};

class ConstantCombinator: public Entity
{
public:
    void tick(CircuitManager & circuits) const override
    {
        port.write(circuits, constants);
    }

    Port port;
    CircuitValues constants;
};

class ArithmeticCombinator: public Entity
{
public:
    enum class Op
    {
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
        POW,
        LSH,
        RSH,
        AND,
        OR,
        XOR
    };

    static_assert(!std::same_as<SignalId, SignalValue>); // permit overload of constructor on these types

    ArithmeticCombinator(SignalId lhs, Op op, SignalId rhs, SignalId out):
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

    ArithmeticCombinator(SignalId lhs, Op op, SignalValue rhs_const, SignalId out):
        _lhs(lhs), _op(op), _rhs(LogicSignal::constant), _out(out), _rhs_const(rhs_const)
    {
        assert(_lhs < num_signals || _lhs == LogicSignal::each);
        assert(_out < num_signals || _out == LogicSignal::each);

        if (_lhs != LogicSignal::each)
        {
            assert(_out != LogicSignal::each);
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

    Port in_port;
    Port out_port;

private:
    SignalId const _lhs;
    Op const _op;
    SignalId const _rhs;
    SignalId const _out;

    SignalValue _rhs_const;

    SignalValue _operate(SignalValue lhs, SignalValue rhs) const
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
};

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

int main()
{
    return 0;
}
