#pragma once

#include "src/Entity.h"

class Machine: public Entity
{
public:
    Machine() {}
    Machine(Machine const &) = delete;

    void tick(CircuitManager & circuits) const override
    {
    }

    Port port;
};