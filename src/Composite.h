#pragma once

#include "Entity.h"
#include "Factorio.h"

class Composite: public Entity
{
public:
    Composite(Composite const &) = delete;

    void tick(CircuitManager & circuits) const override
    {
    }

    int to_blueprint_entity(Blueprint::Entity & bpe) const override
    {
        assert(false); // should not be called on composite entities.
    }

protected:
    Composite(Factorio & factorio): Entity(factorio), _factorio(factorio)
    {
        assert(_primitive_constituents.size() == 1);
        assert(_primitive_constituents.at(0) == this);
        _primitive_constituents.clear();
    }

    template <typename T, typename... Args>
    requires std::derived_from<T, Entity>
    T & _new_entity(Args&&... args)
    {
        T & e = _factorio.new_entity<T>(std::forward<Args>(args)...);
        std::vector<Entity*> const es = e.primitive_constituents();
        _primitive_constituents.insert(_primitive_constituents.end(), es.cbegin(), es.cend());
        _direct_constituents.push_back(&e);
        return e;
    }

    void _connect(WireColor color, Port & a, Port & b)
    {
        _factorio.connect(color, a, b);
    }

private:
    Factorio & _factorio;
};
