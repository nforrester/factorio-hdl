#pragma once

#include "CircuitManager.h"
#include "Port.h"

class Entity
{
public:
    virtual void tick(CircuitManager & circuits) const = 0;
};
