#pragma once

#include <vector>
#include <forward_list>
#include <unordered_map>
#include <unordered_set>

class Entity;

namespace Blueprint {
    struct Blueprint;
    class Entity;

    struct PositionHash
    {
        size_t operator()(std::pair<double, double> const & p) const
        {
            size_t hx = std::hash<double>()(p.first);
            size_t hy = std::hash<double>()(p.second);
            return std::hash<size_t>()(hx ^ hy);
        }
    };

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
            size_t total_non_interface_area = 0;
        };
        std::forward_list<EntityState> entity_states;
        std::unordered_map<::Entity const *, EntityState*> entity_states_by_entity;
        std::unordered_map<Entity*, EntityState*> entity_states_by_blueprint_entity;
        std::unordered_map<int, EntityState*> entity_states_by_id;

		std::unordered_set<std::pair<double, double>, PositionHash> medium_electric_pole_positions;
    };

    double distance_between_entities(
        double x1, double y1,
        int size_x1, int size_y1,
        double x2, double y2,
        int size_x2, int size_y2);

    /* Assumes that all entities are either 1x1 or 1x2. */
    double distance_between_entities(
        double x1, double y1,
        double x2, double y2);

    bool close_enough_for_direct_connection(
        double x1, double y1,
        int size_x1, int size_y1,
        double x2, double y2,
        int size_x2, int size_y2);

    /* Assumes that all entities are either 1x1 or 1x2. */
    bool close_enough_for_direct_connection(
        double x1, double y1,
        double x2, double y2);

    /* Set position and direction for all entities in blueprint.
     * Blueprint should only have combinators.
     * 6 medium electric poles will be added.
     *
     * Constant combinators whose signals are all 0
     * are assumed to mark exernal interfaces. */
    void arrange_blueprint_6x7_cell(
        LayoutState & layout_state,
        std::vector<LayoutState::EntityState*> const & top_cell_entity_states,
        int offset_x_cells,
        int offset_y_cells);
}
