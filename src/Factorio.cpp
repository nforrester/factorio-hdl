#include "Factorio.h"

#include "blueprint/Blueprint.h"
#include "blueprint/layout.h"

void Factorio::connect(WireColor color, Port & a, Port & b)
{
    assert(!_built);

    _planner.connect(color, a, b);
}

bool Factorio::connected(Port const & p) const
{
    return _planner.connected(p);
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

std::string Factorio::get_blueprint_string(Entity const & entity, std::string const & label)
{
    assert(_built);

    Blueprint::Blueprint blueprint;
    Blueprint::LayoutState layout_state(blueprint);

    blueprint.label = label;

    blueprint.icons.emplace_back(1, Blueprint::Signal(Signal::constant_combinator));
    blueprint.icons.emplace_back(2, Blueprint::Signal(Signal::decider_combinator));
    blueprint.icons.emplace_back(3, Blueprint::Signal(Signal::constant_combinator));
    blueprint.icons.emplace_back(4, Blueprint::Signal(Signal::arithmetic_combinator));

    int total_area = 0;
    for (Entity const * e : entity.primitive_constituents())
    {
        Blueprint::Entity be;
        be.id = blueprint.entities.size() + 1;
        total_area += e->to_blueprint_entity(be);
        assert(blueprint.entities.count(be.id) == 0);
        blueprint.entities[be.id] = be;

        layout_state.entity_states.emplace_front();
        Blueprint::LayoutState::EntityState & entity_state = layout_state.entity_states.front();
        entity_state.primitive = true;
        entity_state.entity = e;
        entity_state.blueprint_entity = &blueprint.entities.at(be.id);
        layout_state.entity_states_by_entity[e] = &entity_state;
        layout_state.entity_states_by_blueprint_entity[&blueprint.entities.at(be.id)] = &entity_state;
        layout_state.entity_states_by_id[be.id] = &entity_state;
    }

    std::function<void(::Entity const *)> make_entity_states_for_composite_entities;
    make_entity_states_for_composite_entities =
        [&make_entity_states_for_composite_entities,
         &layout_state]
        (Entity const * e)
        {
            if (layout_state.entity_states_by_entity.count(e))
            {
                return;
            }

            layout_state.entity_states.emplace_front();
            Blueprint::LayoutState::EntityState & entity_state = layout_state.entity_states.front();
            entity_state.entity = e;
            layout_state.entity_states_by_entity[e] = &entity_state;
            for (Entity const * dc : e->direct_constituents())
            {
                make_entity_states_for_composite_entities(dc);
                entity_state.direct_constituents.push_back(layout_state.entity_states_by_entity.at(dc));
            }
        };
    make_entity_states_for_composite_entities(&entity);

    Blueprint::LayoutState::EntityState & top_level_entity_state =
        *layout_state.entity_states_by_entity.at(&entity);

    /* This implements a hub and spoke wiring strategy.
     * It only works for designs that fit in a 6x7 cell.
     * This will need to be upgraded to handle large blueprints. */
    struct BPPortInfo
    {
        int entity_id;
        int port_num;
        Blueprint::Entity::Port * port;
    };
    std::unordered_map<CircuitId, BPPortInfo> hubs;
    std::unordered_map<Port *, BPPortInfo> port_map;
    for (size_t id = 1; id < entity.primitive_constituents().size() + 1; ++id)
    {
        Entity const & e = *entity.primitive_constituents().at(id - 1);
        Blueprint::Entity & be = blueprint.entities.at(id);

        for (auto const & np : e.ports())
        {
            std::string const & name = np.first;
            assert(np.second);
            Port const & p = *np.second;

            int const bp_port_num = e.ports().size() == 1 ? 1 : name == "in" ? 1 : 2;
            assert(be.ports.count(bp_port_num) == 0);
            auto & bpp = be.ports[bp_port_num] = Blueprint::Entity::Port();

            assert(port_map.count(const_cast<Port*>(&p)) == 0);
            port_map[const_cast<Port*>(&p)] = {be.id, bp_port_num, &bpp};

            for (WireColor w = 0; w < num_wire_colors; w++)
            {
                CircuitId cid = p.circuit_id(w);
                if (cid == invalid_circuit_id)
                {
                    continue;
                }
                if (hubs.count(cid) == 0)
                {
                    hubs[cid] = {be.id, bp_port_num, &bpp};
                }
                else
                {
                    BPPortInfo const & bpi = hubs.at(cid);

                    assert(w == Wire::red || w == Wire::green);
                    auto & bpp_wires = w == Wire::red ? bpp.red : bpp.green;
                    auto & hub_wires = w == Wire::red ? bpi.port->red : bpi.port->green;

                    bpp_wires.emplace_back(bpi.entity_id, bpi.port_num);
                    hub_wires.emplace_back(be.id, bp_port_num);
                }
            }
        }
    }

    /* For each interface, add a constant combinator with a label and a wire. */
    for (auto const & np : entity.ports())
    {
        std::string const & name = np.first;
        Port const * port = np.second;
        assert(port);

        Blueprint::Entity be;
        be.id = blueprint.entities.size() + 1;

        be.name = Signal::constant_combinator;
        be.control_behavior = Blueprint::Entity::Filters();
        Blueprint::Entity::Filters & f = std::get<Blueprint::Entity::Filters>(*be.control_behavior);
        size_t i = 0;
        for (char c : name)
        {
            if ('A' <= c && c <= 'Z')
            {
                c += 'a' - 'A';
            }
            if (c < 'a' || 'z' < c)
            {
                continue;
            }
            if (i == 4)
            {
                break;
            }
            ++i;
            f.filters.emplace_back(0, i, static_cast<SignalId>(c - 'a') + Signal::letter_a);
        }

        Blueprint::Entity::Port interface_bpp;
        BPPortInfo const & pmpi = port_map.at(const_cast<Port*>(port));
        assert(pmpi.port);

        for (WireColor w : entity.port_interface_colors(*port))
        {
            assert(w == Wire::red || w == Wire::green);
            auto & bpp_wires = w == Wire::red ? pmpi.port->red : pmpi.port->green;
            auto & int_wires = w == Wire::red ? interface_bpp.red : interface_bpp.green;

            bpp_wires.emplace_back(be.id, 1);
            int_wires.emplace_back(pmpi.entity_id, pmpi.port_num);
        }

        be.ports[1] = interface_bpp;

        assert(blueprint.entities.count(be.id) == 0);
        blueprint.entities[be.id] = be;

        layout_state.entity_states.emplace_front();
        Blueprint::LayoutState::EntityState & es = layout_state.entity_states.front();
        es.primitive = true;
        es.blueprint_entity = &blueprint.entities.at(be.id);
        layout_state.entity_states_by_blueprint_entity[es.blueprint_entity] = &es;
        layout_state.entity_states_by_id[be.id] = &es;
        top_level_entity_state.direct_constituents.push_back(&es);
    }

    if (total_area <= 42)
    {
        arrange_blueprint_6x7_cell(layout_state, top_level_entity_state, 0, 0);
    }
    else
    {
        throw std::runtime_error("Too big: " + std::to_string(total_area));
    }

    for (auto const & es : layout_state.entity_states)
    {
        assert(es.placed);
    }

    return blueprint.to_blueprint_string();
}
