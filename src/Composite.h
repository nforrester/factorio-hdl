#pragma once

#include "Entity.h"
#include "Factorio.h"

class Composite: public Entity
{
public:
    Composite(Composite const &) = delete;

    void tick(CircuitManager & circuits) const
    {
    }

protected:
    Composite(Factorio & factorio): Entity(factorio), _factorio(factorio)
    {
    }

    template <typename T, typename... Args>
    requires std::derived_from<T, Entity>
    T & _new_entity(Args&&... args)
    {
        return _factorio.new_entity<T>(std::forward<Args>(args)...);
    }

    void _connect(WireColor color, Port & a, Port & b)
    {
        _factorio.connect(color, a, b);
    }

    void _lock(WireColor color, Port & p)
    {
        _factorio.lock(color, p);
    }

private:
    Factorio & _factorio;
};
