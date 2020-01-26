#pragma once

#include "Entity.h"

class ConstantCombinator: public Entity
{
public:
    ConstantCombinator() {}
    ConstantCombinator(ConstantCombinator const &) = delete;

    void tick(CircuitManager & circuits) const override
    {
        port.write(circuits, constants);
    }

    Port port;
    CircuitValues constants;
};
