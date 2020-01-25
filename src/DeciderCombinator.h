#pragma once

#include "Entity.h"
#include "Port.h"

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

    DeciderCombinator(SignalId lhs, Op op, SignalId rhs, SignalId out, bool write_input_count);
    DeciderCombinator(SignalId lhs, Op op, SignalValue rhs_const, SignalId out, bool write_input_count);

    void tick(CircuitManager & circuits) const override;

    Port in_port;
    Port out_port;

private:
    SignalId const _lhs;
    Op const _op;
    SignalId const _rhs;
    SignalId const _out;

    SignalValue _rhs_const;

    bool _write_input_count;

    bool _operate(SignalValue lhs, SignalValue rhs) const;
};
