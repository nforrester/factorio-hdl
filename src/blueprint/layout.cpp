#include "layout.h"
#include "Blueprint.h"

#include <unordered_set>

namespace
{
    /* Constant combinators whose signals are all 0
     * are assumed to mark exernal interfaces. */
    bool is_interface(Blueprint::Entity const & e)
    {
        if (e.name != Signal::constant_combinator)
        {
            return false;
        }
        assert(e.control_behavior.has_value());
        for (auto const & f : std::get<Blueprint::Entity::Filters>(*e.control_behavior).filters)
        {
            if (f.count != 0)
            {
                return false;
            }
        }
        return true;
    }
}

void Blueprint::arrange_blueprint_6x7_cell(
    LayoutState & layout_state,
    LayoutState::EntityState & entity_state_for_whole_cell,
    int offset_x_cells,
    int offset_y_cells)
{
    int offset_x = offset_x_cells * 7;
    int offset_y = offset_y_cells * 8;

    std::vector<LayoutState::EntityState*> primitive_entities_for_this_cell;
    std::function<void(LayoutState::EntityState *)> list_primitive_entities_for_this_cell;
    list_primitive_entities_for_this_cell =
        [&list_primitive_entities_for_this_cell,
         &primitive_entities_for_this_cell]
        (LayoutState::EntityState * es)
        {
            if (es->primitive)
            {
                primitive_entities_for_this_cell.push_back(es);
            }
            for (LayoutState::EntityState * dc : es->direct_constituents)
            {
                list_primitive_entities_for_this_cell(dc);
            }
        };
    list_primitive_entities_for_this_cell(&entity_state_for_whole_cell);

    /* 7 slots on one side, minus 1 for a power pole.
     * The other side is reserved for the next 6x7 cell. */
    size_t constexpr max_interface_slots = (7 - 1) * 1;
    std::vector<int> interface_entity_ids;

    struct Slot1x2
    {
        int area_free = 2;
        std::vector<int> entity_ids;
    };
    size_t constexpr max_slots = 7 * 3;
    std::array<Slot1x2, max_slots> slots;

    size_t next_2_slot = 0;
    size_t next_1_slot = 0;

    for (LayoutState::EntityState const * es : primitive_entities_for_this_cell)
    {
        Entity const & e = *es->blueprint_entity;
        assert(!e.position.has_value());

        if (is_interface(e))
        {
            interface_entity_ids.push_back(e.id);
            assert(interface_entity_ids.size() <= max_interface_slots);
            continue;
        }

        if (e.name == ::Signal::constant_combinator)
        {
            assert(next_1_slot < slots.size());
            Slot1x2 & slot = slots.at(next_1_slot);
            assert(slot.area_free >= 1);
            slot.entity_ids.push_back(e.id);
            slot.area_free -= 1;
            if (slot.area_free == 0)
            {
                next_1_slot = next_2_slot;
            }
            else if (slot.area_free == 1)
            {
                assert(next_1_slot == next_2_slot);
                ++next_2_slot;
            }
        }
        else
        {
            assert(e.name == ::Signal::decider_combinator ||
                   e.name == ::Signal::arithmetic_combinator);
            assert(next_2_slot < slots.size());
            Slot1x2 & slot = slots.at(next_2_slot);
            assert(slot.area_free == 2);
            slot.entity_ids.push_back(e.id);
            slot.area_free -= 2;
            assert(slot.area_free == 0);
            if (next_1_slot == next_2_slot)
            {
                ++next_1_slot;
            }
            ++next_2_slot;
        }
    }

    size_t constexpr num_power_poles = 6;
    std::array<Entity::Position, num_power_poles> power_pole_positions =
    {
        Entity::Position(0,0),
        Entity::Position(0,4),
        Entity::Position(0,8),
        Entity::Position(7,0),
        Entity::Position(7,4),
        Entity::Position(7,8),
    };

    std::array<Entity::Position, max_interface_slots> interface_positions =
    {
        Entity::Position(0,1),
        Entity::Position(0,2),
        Entity::Position(0,3),
        Entity::Position(0,5),
        Entity::Position(0,6),
        Entity::Position(0,7),
    };

    std::array<Entity::Position, max_slots> slot_positions =
    {
        Entity::Position(1,1),
        Entity::Position(1,2),
        Entity::Position(1,3),
        Entity::Position(1,4),
        Entity::Position(1,5),
        Entity::Position(1,6),
        Entity::Position(1,7),
        Entity::Position(3,1),
        Entity::Position(3,2),
        Entity::Position(3,3),
        Entity::Position(3,4),
        Entity::Position(3,5),
        Entity::Position(3,6),
        Entity::Position(3,7),
        Entity::Position(5,1),
        Entity::Position(5,2),
        Entity::Position(5,3),
        Entity::Position(5,4),
        Entity::Position(5,5),
        Entity::Position(5,6),
        Entity::Position(5,7),
    };

    for (size_t i = 0; i < num_power_poles; ++i)
    {
        Entity pole;
        pole.position = power_pole_positions.at(i);
        pole.position->x += offset_x;
        pole.position->y += offset_y;
        auto ppp = std::make_pair(pole.position->x, pole.position->y);
        if (layout_state.medium_electric_pole_positions.count(ppp))
        {
            continue;
        }
        layout_state.medium_electric_pole_positions.insert(ppp);

        pole.name = ::Signal::medium_electric_pole;
        pole.id = layout_state.blueprint.entities.size() + 1;
        assert(layout_state.blueprint.entities.count(pole.id) == 0);
        layout_state.blueprint.entities[pole.id] = pole;

        layout_state.entity_states.emplace_front();
        LayoutState::EntityState & pole_state = layout_state.entity_states.front();
        pole_state.blueprint_entity = &layout_state.blueprint.entities.at(pole.id);
        pole_state.placed = true;
        layout_state.entity_states_by_blueprint_entity[pole_state.blueprint_entity] = &pole_state;
        layout_state.entity_states_by_id[pole.id] = &pole_state;
        entity_state_for_whole_cell.direct_constituents.push_back(&pole_state);
    }

    for (size_t i = 0; i < interface_entity_ids.size(); ++i)
    {
        Entity & e = layout_state.blueprint.entities.at(interface_entity_ids.at(i));
        e.direction = 1;
        e.position = interface_positions.at(i);
        e.position->x += offset_x;
        e.position->y += offset_y;
        layout_state.entity_states_by_id.at(interface_entity_ids.at(i))->placed = true;
    }

    for (size_t i = 0; i < max_slots; ++i)
    {
        Slot1x2 const & slot = slots.at(i);
        for (size_t j = 0; j < slot.entity_ids.size(); ++j)
        {
            Entity & e = layout_state.blueprint.entities.at(slot.entity_ids.at(j));
            e.direction = 2;
            e.position = slot_positions.at(i);
            e.position->x += offset_x;
            e.position->y += offset_y;
            if (e.name != ::Signal::constant_combinator)
            {
                assert(e.name == ::Signal::decider_combinator ||
                       e.name == ::Signal::arithmetic_combinator);
                assert(j == 0);
                e.position->x += 0.5;
            }
            else
            {
                e.position->x += j;
            }
            layout_state.entity_states_by_id.at(slot.entity_ids.at(j))->placed = true;
        }
    }

    std::function<void(LayoutState::EntityState *)> mark_all_as_placed;
    mark_all_as_placed =
        [&mark_all_as_placed]
        (LayoutState::EntityState * es)
        {
            for (LayoutState::EntityState * dc : es->direct_constituents)
            {
                mark_all_as_placed(dc);
            }
            es->placed = true;
        };
    mark_all_as_placed(&entity_state_for_whole_cell);
}
