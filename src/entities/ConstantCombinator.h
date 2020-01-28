#pragma once

#include "src/Entity.h"

class ConstantCombinator: public Entity
{
public:
    ConstantCombinator(Factorio & factorio): Entity(factorio)
    {
        _set_port("out", _port);
    }

    ConstantCombinator(ConstantCombinator const &) = delete;

    void tick(CircuitManager & circuits) const override
    {
        _port.write(circuits, constants);
    }

    CircuitValues constants;

private:
    Port _port;
};
