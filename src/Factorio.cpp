#include "Factorio.h"

void Factorio::connect(WireColor color, Port & a, Port & b)
{
    assert(!_built);

    _planner.connect(color, a, b);
}

void Factorio::lock(WireColor color, Port & p)
{
    assert(!_built);

    _planner.lock(color, p);
}

void Factorio::build()
{
    assert(!_built);
    _built = true;

    _planner.build(_circuits);
}

bool Factorio::tick()
{
    assert(_built);

    for (auto & entity : _entities)
    {
        entity->tick(_circuits);
    }
    bool stable = _circuits.stable();
    _circuits.finish_tick();
    return stable;
}

bool Factorio::run_until_stable(size_t timeout)
{
    assert(_built);

    bool stable = false;
    for (size_t i = 0; i < timeout && !stable; ++i)
    {
        stable = tick();
    }

    return stable;
}
