#pragma once

#include "CircuitManager.h"

class Entity
{
public:
    virtual void tick(CircuitManager & circuits) const = 0;
};
