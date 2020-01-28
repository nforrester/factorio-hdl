#pragma once

#include "src/Entity.h"

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

    DeciderCombinator(Factorio & factorio, SignalId lhs, Op op, SignalId rhs, SignalId out, bool write_input_count);
    DeciderCombinator(Factorio & factorio, SignalId lhs, Op op, SignalValue rhs_const, SignalId out, bool write_input_count);
    DeciderCombinator(DeciderCombinator const &) = delete;

    void tick(CircuitManager & circuits) const override;

private:
    Port _in_port;
    Port _out_port;

    void _common_init();

    SignalId const _lhs;
    Op const _op;
    SignalId const _rhs;
    SignalId const _out;

    SignalValue _rhs_const;

    bool _write_input_count;

    bool _operate(SignalValue lhs, SignalValue rhs) const;
};
