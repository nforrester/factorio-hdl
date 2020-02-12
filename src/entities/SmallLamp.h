#pragma once

#include "src/Entity.h"
#include "DeciderCombinator.h"

class SmallLamp: public Entity
{
public:
    static_assert(!std::same_as<SignalId, SignalValue>); // permit overload of constructor on these types

    SmallLamp(Factorio & factorio,
              std::string const & log_leader,
              SignalId lhs,
              DeciderCombinator::Op op,
              SignalId rhs,
              bool use_colors);

    SmallLamp(Factorio & factorio,
              std::string const & log_leader,
              SignalId lhs,
              DeciderCombinator::Op op,
              SignalValue rhs_const,
              bool use_colors);

    SmallLamp(SmallLamp const &) = delete;

    void tick(CircuitManager & circuits) override;

    int to_blueprint_entity(Blueprint::Entity & bpe) const override;

    SignalId color() const
    {
        return _color;
    }

private:
    Port _in_port;

    void _common_init();

    SignalId const _lhs;
    DeciderCombinator::Op const _op;
    SignalId const _rhs;

    SignalValue _rhs_const;

    bool _use_colors;

    SignalId _color = Signal::signal_black;
};
