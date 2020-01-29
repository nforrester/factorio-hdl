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

        std::unique_ptr<T> ptr = std::make_unique<T>(*this, std::forward<Args>(args)...);
        T & ref = *ptr;
        _entities.push_back(std::move(ptr));
        return ref;
    }

    void connect(WireColor color, Port & a, Port & b);
    void lock(WireColor color, Port & p);
    bool locked(WireColor color, Port & p) const;
    void build();
    bool tick();
    bool run_until_stable(size_t timeout);

    // Get a blueprint for the given Entity.
    std::string get_blueprint_string(Entity const & entity);

    CircuitValues read(Port const & port) const
    {
        return port.read(_circuits);
    }

    CircuitValues const & read(Port const & port, WireColor color) const
    {
        return port.read(_circuits, color);
    }

private:
    bool _built = false;

    CircuitPlanner _planner;

    CircuitManager _circuits;

    std::vector<std::unique_ptr<Entity>> _entities;
};
