#pragma once

#include "src/Entity.h"
#include "src/debug.h"

class ConstantCombinator: public Entity
{
public:
    ConstantCombinator(Factorio & factorio, std::string const & log_leader):
        Entity(factorio, log_leader),
        _port(log_leader + "out > ")
    {
        _set_port("out", _port);
    }

    ConstantCombinator(ConstantCombinator const &) = delete;

    void tick(CircuitManager & circuits) const override
    {
        debug(1) << _log_leader << "Write " << constants << "\n";
        _port.write(circuits, constants);
    }

    CircuitValues constants;

    int to_blueprint_entity(Blueprint::Entity & bpe) const override;

private:
    Port _port;
};
