#pragma once

#include <vector>
#include <forward_list>
#include <unordered_map>

class Entity;

namespace Blueprint {
    struct Blueprint;
    class Entity;

    void move_blueprint(Blueprint & blueprint, double x, double y);
    void merge_blueprints(Blueprint & dst, Blueprint const & src);

    struct LayoutState
    {
        LayoutState(Blueprint & blueprint_): blueprint(blueprint_) {}
        Blueprint & blueprint;
        struct EntityState
        {
            ::Entity const * entity = nullptr;
            Entity * blueprint_entity = nullptr;
            bool placed = false;
            std::vector<EntityState*> direct_constituents;
            bool primitive = false;
        };
        std::forward_list<EntityState> entity_states;
        std::unordered_map<::Entity const *, EntityState*> entity_states_by_entity;
        std::unordered_map<Entity*, EntityState*> entity_states_by_blueprint_entity;
        std::unordered_map<int, EntityState*> entity_states_by_id;
    };

    /* Set position and direction for all entities in blueprint.
     * Blueprint should only have combinators.
     * 6 medium electric poles will be added.
     *
     * Constant combinators whose signals are all 0
     * are assumed to mark exernal interfaces. */
    void arrange_blueprint_6x7_cell(
        LayoutState & layout_state,
        LayoutState::EntityState & entity_state_for_whole_cell);
}
