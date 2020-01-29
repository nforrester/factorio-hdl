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

    void to_blueprint_entity(Blueprint::Entity & bpe) const override
    {
        assert(false); // should not be called on composite entities.
    }

protected:
    Composite(Factorio & factorio): Entity(factorio), _factorio(factorio)
    {
        assert(_constituent_entities.size() == 1);
        assert(_constituent_entities.at(0) == this);
        _constituent_entities.clear();
    }

    template <typename T, typename... Args>
    requires std::derived_from<T, Entity>
    T & _new_entity(Args&&... args)
    {
        T & e = _factorio.new_entity<T>(std::forward<Args>(args)...);
        std::vector<Entity*> const es = e.constituent_entities();
        _constituent_entities.insert(_constituent_entities.end(), es.cbegin(), es.cend());
        return e;
    }

    void _connect(WireColor color, Port & a, Port & b)
    {
        _factorio.connect(color, a, b);
    }

private:
    Factorio & _factorio;
};
