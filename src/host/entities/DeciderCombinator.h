#pragma once

#include "src/host/Entity.h"

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

    DeciderCombinator(Factorio & factorio,
                      std::string const & log_leader,
                      SignalId lhs,
                      Op op,
                      SignalId rhs,
                      SignalId out,
                      bool write_input_count);

    DeciderCombinator(Factorio & factorio,
                      std::string const & log_leader,
                      SignalId lhs,
                      Op op,
                      SignalValue rhs_const,
                      SignalId out,
                      bool write_input_count);

    DeciderCombinator(DeciderCombinator const &) = delete;

    void tick(CircuitManager & circuits) override;

    int to_blueprint_entity(Blueprint::Entity & bpe) const override;

    static bool operate(SignalValue lhs, Op op, SignalValue rhs);

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
