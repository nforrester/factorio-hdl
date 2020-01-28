#include "layout.h"
#include "Blueprint.h"

#include <unordered_set>

void move_blueprint(Blueprint & blueprint, double x, double y)
{
    for (auto & ie : blueprint.entities)
    {
        Blueprint::Entity & e = ie.second;
        assert(e.position.has_value());
        e.position->x += x;
        e.position->y += y;
    }
}

namespace
{
    void assert_only_legal_entities(Blueprint const & blueprint)
    {
        // Code depends on legal entities being restricted to either 1x1 or 1x2.
        static std::unordered_set<SignalId> const legal_entities =
        {
            Signal::medium_electric_pole,
            Signal::constant_combinator,
            Signal::arithmetic_combinator,
            Signal::decider_combinator,
        };

        for (auto const & ie : blueprint.entities)
        {
            Blueprint::Entity const & e = ie.second;
            assert(legal_entities.count(e.name) == 1);
        }
    }

    struct CollisionSet
    {
        CollisionSet(Blueprint const & blueprint)
        {
            // Ensure that all entities are 1x1 or 1x2.
            assert_only_legal_entities(blueprint);

            for (auto const & ie : blueprint.entities)
            {
                Blueprint::Entity const & e = ie.second;
                bool x_int = e.position->x == fmod(e.position->x, 1);
                bool y_int = e.position->y == fmod(e.position->y, 1);
                assert(x_int || y_int);
                if (x_int)
                {
                    if (y_int)
                    {
                        if (e.name == Signal::medium_electric_pole)
                        {
                            medium_electric_pole_positions.insert(*e.position);
                        }
                        else
                        {
                            non_merging_positions.insert(*e.position);
                        }
                    }
                    else
                    {
                        assert(x_int && (!y_int));
                        assert(e.name != Signal::medium_electric_pole);
                        double const x = e.position->x;
                        double const y = e.position->y;
                        double const y1 = floor(y);
                        double const y2 = y1 + 1;
                        non_merging_positions.insert(Blueprint::Entity::Position(x, y1));
                        non_merging_positions.insert(Blueprint::Entity::Position(x, y2));
                    }
                }
                else
                {
                    assert((!x_int) && y_int);
                    assert(e.name != Signal::medium_electric_pole);
                    double const x = e.position->x;
                    double const y = e.position->y;
                    double const x1 = floor(x);
                    double const x2 = x1 + 1;
                    non_merging_positions.insert(Blueprint::Entity::Position(x1, y));
                    non_merging_positions.insert(Blueprint::Entity::Position(x2, y));
                }
            }
        }

        struct PositionHash
        {
            size_t operator()(Blueprint::Entity::Position const & p) const
            {
                size_t hx = std::hash<double>()(p.x);
                size_t hy = std::hash<double>()(p.y);
                return std::hash<size_t>()(hx ^ hy);
            }
        };

        std::unordered_set<Blueprint::Entity::Position, PositionHash> non_merging_positions;
        std::unordered_set<Blueprint::Entity::Position, PositionHash> medium_electric_pole_positions;
    };
}

void merge_blueprints(Blueprint & dst, Blueprint const & src)
{
    CollisionSet dst_collisions(dst);
    CollisionSet src_collisions(src);

    for (auto const & p : src_collisions.non_merging_positions)
    {
        assert(dst_collisions.non_merging_positions.count(p) == 0);
        assert(dst_collisions.medium_electric_pole_positions.count(p) == 0);
    }

    for (auto const & p : src_collisions.medium_electric_pole_positions)
    {
        assert(dst_collisions.non_merging_positions.count(p) == 0);
    }

    assert(false);
    // TODO ACTUALLY PERFORM THE MERGE. BE SURE TO CHANGE WIRE TARGET ENTITY IDS.
//    for (auto const & ie : src.entities)
//    {
//        Blueprint::Entity const & e = ie.second;
//        if (dst_collisions.medium_electric_pole_positions.count(*e.p) == 0)
//        {
//        }
//    }
}

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

/* Set position and direction for all entities in blueprint.
 * Blueprint should only have combinators.
 * 6 medium electric poles will be added. */
void arrange_blueprint_6x7_cell(Blueprint & blueprint)
{
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

    for (auto const & ie : blueprint.entities)
    {
        Blueprint::Entity const & e = ie.second;
        assert(!e.position.has_value());

        if (is_interface(e))
        {
            interface_entity_ids.push_back(e.id);
            assert(interface_entity_ids.size() <= max_interface_slots);
            continue;
        }

        if (e.name == Signal::constant_combinator)
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
            assert(e.name == Signal::decider_combinator ||
                   e.name == Signal::arithmetic_combinator);
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
    std::array<Blueprint::Entity::Position, num_power_poles> power_pole_positions =
    {
        Blueprint::Entity::Position(0,0),
        Blueprint::Entity::Position(0,4),
        Blueprint::Entity::Position(0,8),
        Blueprint::Entity::Position(7,0),
        Blueprint::Entity::Position(7,4),
        Blueprint::Entity::Position(7,8),
    };

    std::array<Blueprint::Entity::Position, max_interface_slots> interface_positions =
    {
        Blueprint::Entity::Position(0,1),
        Blueprint::Entity::Position(0,2),
        Blueprint::Entity::Position(0,3),
        Blueprint::Entity::Position(0,5),
        Blueprint::Entity::Position(0,6),
        Blueprint::Entity::Position(0,7),
    };

    std::array<Blueprint::Entity::Position, max_slots> slot_positions =
    {
        Blueprint::Entity::Position(1,1),
        Blueprint::Entity::Position(1,2),
        Blueprint::Entity::Position(1,3),
        Blueprint::Entity::Position(1,4),
        Blueprint::Entity::Position(1,5),
        Blueprint::Entity::Position(1,6),
        Blueprint::Entity::Position(1,7),
        Blueprint::Entity::Position(3,1),
        Blueprint::Entity::Position(3,2),
        Blueprint::Entity::Position(3,3),
        Blueprint::Entity::Position(3,4),
        Blueprint::Entity::Position(3,5),
        Blueprint::Entity::Position(3,6),
        Blueprint::Entity::Position(3,7),
        Blueprint::Entity::Position(5,1),
        Blueprint::Entity::Position(5,2),
        Blueprint::Entity::Position(5,3),
        Blueprint::Entity::Position(5,4),
        Blueprint::Entity::Position(5,5),
        Blueprint::Entity::Position(5,6),
        Blueprint::Entity::Position(5,7),
    };

    for (size_t i = 0; i < num_power_poles; ++i)
    {
        Blueprint::Entity pole;
        pole.name = Signal::medium_electric_pole;
        pole.position = power_pole_positions.at(i);
        // TODO FINISH pole
        pole.id = blueprint.entities.size();
        assert(blueprint.entities.count(pole.id) == 0);
        blueprint.entities[pole.id] = pole;
    }

    for (size_t i = 0; i < interface_entity_ids.size(); ++i)
    {
        Blueprint::Entity & e = blueprint.entities.at(interface_entity_ids.at(i));
        e.direction = 0;
        e.position = interface_positions.at(i);
    }

    for (size_t i = 0; i < max_slots; ++i)
    {
        Slot1x2 const & slot = slots.at(i);
        for (size_t j = 0; j < slot.entity_ids.size(); j++)
        {
            Blueprint::Entity & e = blueprint.entities.at(slot.entity_ids.at(j));
            e.direction = 0;
            e.position = slot_positions.at(j);
            if (e.name != Signal::constant_combinator)
            {
                assert(e.name == Signal::decider_combinator ||
                       e.name == Signal::arithmetic_combinator);
                assert(j == 0);
                e.position->x += 0.5;
            }
            else
            {
                e.position->x += j;
            }
        }
    }
}