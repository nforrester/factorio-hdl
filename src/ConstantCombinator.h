#pragma once

#include "Entity.h"

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
