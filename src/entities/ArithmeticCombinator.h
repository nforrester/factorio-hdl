#pragma once

#include "src/Entity.h"

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
        XOR,
    };

    static_assert(!std::same_as<SignalId, SignalValue>); // permit overload of constructor on these types

    ArithmeticCombinator(Factorio & factorio, SignalId lhs, Op op, SignalId rhs, SignalId out);
    ArithmeticCombinator(Factorio & factorio, SignalId lhs, Op op, SignalValue rhs_const, SignalId out);
    ArithmeticCombinator(ArithmeticCombinator const &) = delete;

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

    SignalValue _operate(SignalValue lhs, SignalValue rhs) const;
};
