#pragma once

#include "Entity.h"
#include "Port.h"

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

    ArithmeticCombinator(SignalId lhs, Op op, SignalId rhs, SignalId out);
    ArithmeticCombinator(SignalId lhs, Op op, SignalValue rhs_const, SignalId out);

    void tick(CircuitManager & circuits) const override;

    Port in_port;
    Port out_port;

private:
    SignalId const _lhs;
    Op const _op;
    SignalId const _rhs;
    SignalId const _out;

    SignalValue _rhs_const;

    SignalValue _operate(SignalValue lhs, SignalValue rhs) const;
};
