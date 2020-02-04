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
        std::cout << "CONST: -> " << constants << "\n";
        _port.write(circuits, constants);
    }

    CircuitValues constants;

    int to_blueprint_entity(Blueprint::Entity & bpe) const override;

private:
    Port _port;
};
