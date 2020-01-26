#pragma once

#include "CircuitPlanner.h"
#include "CircuitManager.h"
#include "Entity.h"

#include <memory>

class Factorio
{
public:
    template <typename T, typename... Args>
    requires std::derived_from<T, Entity>
    T & new_entity(Args&&... args)
    {
        assert(!_built);

        std::unique_ptr<T> ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T & ref = *ptr;
        _entities.push_back(std::move(ptr));
        return ref;
    }

    void connect(WireColor color, Port & a, Port & b)
    {
        assert(!_built);

        _planner.connect(color, a, b);
    }

    void build()
    {
        assert(!_built);
        _built = true;

        _planner.build(_circuits);
    }

    void tick()
    {
        assert(_built);

        for (auto & entity : _entities)
        {
            entity->tick(_circuits);
        }
        _circuits.finish_tick();
    }

    CircuitValues read(Port const & port)
    {
        return port.read(_circuits);
    }

private:
    bool _built = false;

    CircuitPlanner _planner;

    CircuitManager _circuits;

    std::vector<std::unique_ptr<Entity>> _entities;
};
