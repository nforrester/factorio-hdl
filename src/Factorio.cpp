#include "Factorio.h"

#include "blueprint/Blueprint.h"
#include "blueprint/layout.h"
#include "blueprint/hilbert_curve.h"

#include <sstream>
#include <list>

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

    for (Entity const * e : entity.primitive_constituents())
    {
        Blueprint::Entity be;
        be.id = blueprint.entities.size() + 1;
        int const total_non_interface_area = e->to_blueprint_entity(be);
        assert(blueprint.entities.count(be.id) == 0);
        blueprint.entities[be.id] = be;

        layout_state.entity_states.emplace_front();
        Blueprint::LayoutState::EntityState & entity_state = layout_state.entity_states.front();
        entity_state.primitive = true;
        entity_state.entity = e;
        entity_state.blueprint_entity = &blueprint.entities.at(be.id);
        entity_state.total_non_interface_area = total_non_interface_area;
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
            entity_state.total_non_interface_area = 0;
            layout_state.entity_states_by_entity[e] = &entity_state;
            for (Entity const * dc : e->direct_constituents())
            {
                make_entity_states_for_composite_entities(dc);
                entity_state.direct_constituents.push_back(layout_state.entity_states_by_entity.at(dc));
                entity_state.total_non_interface_area +=
                    entity_state.direct_constituents.back()->total_non_interface_area;
            }
        };
    make_entity_states_for_composite_entities(&entity);

    Blueprint::LayoutState::EntityState & top_level_entity_state =
        *layout_state.entity_states_by_entity.at(&entity);

    /* For each interface, add a constant combinator with a label. */
    std::map<int, Port const *> interface_id_to_port_map;
    for (auto const & np : entity.ports())
    {
        std::string const & name = np.first;
        Port const * port = np.second;
        assert(port);

        Blueprint::Entity be;
        be.id = blueprint.entities.size() + 1;
        be.name = Signal::constant_combinator;

        // set label
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

        assert(blueprint.entities.count(be.id) == 0);
        blueprint.entities[be.id] = be;
        interface_id_to_port_map[be.id] = port;

        layout_state.entity_states.emplace_front();
        Blueprint::LayoutState::EntityState & es = layout_state.entity_states.front();
        es.primitive = true;
        es.blueprint_entity = &blueprint.entities.at(be.id);
        layout_state.entity_states_by_blueprint_entity[es.blueprint_entity] = &es;
        layout_state.entity_states_by_id[be.id] = &es;
        top_level_entity_state.direct_constituents.push_back(&es);
    }

    /* Traverse the tree of entities depth first and pack entities into a row of 6x7 cells. */
    std::vector<std::vector<Blueprint::LayoutState::EntityState*>> entities_that_fit_in_cells;
    entities_that_fit_in_cells.emplace_back();
    size_t constexpr cell_area = 6 * 7;
    size_t current_cell_available_area = cell_area;
    std::forward_list<Blueprint::LayoutState::EntityState*> entities_that_might_not_fit_in_cells;
    entities_that_might_not_fit_in_cells.push_front(&top_level_entity_state);
    while (!entities_that_might_not_fit_in_cells.empty())
    {
        Blueprint::LayoutState::EntityState * this_one = entities_that_might_not_fit_in_cells.front();
        entities_that_might_not_fit_in_cells.pop_front();
        if (this_one->total_non_interface_area <= current_cell_available_area)
        {
            entities_that_fit_in_cells.back().push_back(this_one);
            current_cell_available_area -= this_one->total_non_interface_area;
            continue;
        }
        /* Don't break up this entity if it will fit completely in the next cell. */
        if (this_one->total_non_interface_area <= cell_area)
        {
            current_cell_available_area = cell_area;
            entities_that_fit_in_cells.emplace_back();
            entities_that_fit_in_cells.back().push_back(this_one);
            current_cell_available_area -= this_one->total_non_interface_area;
            continue;
        }
        for (Blueprint::LayoutState::EntityState * dc : this_one->direct_constituents)
        {
            entities_that_might_not_fit_in_cells.push_front(dc);
        }
    }

    /* Arrange the row of 6x7 cells on a Hilbert Curve so that adjacent cells will be in close proximity. */
    int const hilbert_curve_final_side_length =
        compute_final_side_length(entities_that_fit_in_cells.size());
    for (size_t i = 0; i < entities_that_fit_in_cells.size(); ++i)
    {
        int x, y;
        std::tie(x, y) = compute_position_along_hilbert_curve(hilbert_curve_final_side_length, i);
        arrange_blueprint_6x7_cell(layout_state, entities_that_fit_in_cells.at(i), x, y);
    }
    for (auto const & es : layout_state.entity_states)
    {
        assert((!es.primitive) || es.placed);
    }

    /* Mark down all the tiles that are already occupied. */
    struct LocationHash
    {
        size_t operator()(std::pair<int, int> const & p) const
        {
            size_t hx = std::hash<int>()(p.first);
            size_t hy = std::hash<int>()(p.second);
            return std::hash<size_t>()(hx ^ hy);
        }
    };
    std::unordered_set<std::pair<int, int>, LocationHash> occupied_locations;
    for (auto const & id_be : blueprint.entities)
    {
        /* Assumes that all entities are either 1x1 or 1x2. */
        Blueprint::Entity const & be = id_be.second;
        int size_x = fabs(fmod(be.position->x, 1.0)) > 0.1 ? 2 : 1;
        int size_y = fabs(fmod(be.position->y, 1.0)) > 0.1 ? 2 : 1;
        assert(size_x + size_y < 4);
        int x = std::floor(be.position->x);
        int y = std::floor(be.position->y);
        assert(occupied_locations.emplace(x, y).second);
        if (size_x == 2)
        {
            assert(occupied_locations.emplace(x + 1, y).second);
        }
        if (size_y == 2)
        {
            assert(occupied_locations.emplace(x, y + 1).second);
        }
    }

    /* This is an iterative wiring strategy. We go over and over the list of entities,
     * trying to connect them to existing connected entities. If we completely stop
     * making progress we then start adding blank constant combinators to bridge the
     * gaps. */
    struct BPPortInfo
    {
        int entity_id;
        int port_num;
        Blueprint::Entity::Port * port;
        std::vector<std::pair<CircuitId, WireColor>> circuits;
    };
    std::list<BPPortInfo> ports_to_wire_up;
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
            be.ports[bp_port_num] = Blueprint::Entity::Port();
            auto & bpp = be.ports.at(bp_port_num);

            BPPortInfo bppi;
            bppi.entity_id = id;
            bppi.port_num = bp_port_num;
            bppi.port = &bpp;

            for (WireColor w = 0; w < num_wire_colors; w++)
            {
                CircuitId cid = p.circuit_id(w);
                if (cid != invalid_circuit_id)
                {
                    bppi.circuits.emplace_back(cid, w);
                }
            }

            ports_to_wire_up.push_back(bppi);

            assert(port_map.count(const_cast<Port*>(&p)) == 0);
            port_map[const_cast<Port*>(&p)] = bppi;
        }
    }

    std::unordered_map<CircuitId, std::vector<BPPortInfo>> network_membership;
    std::set<std::tuple<size_t, int, CircuitId>> unwired_ports;
    std::set<std::tuple<size_t, int, CircuitId>> wired_ports;
    bool progress;
    bool take_desperate_measures = false;
    do
    {
        progress = false;
        for (auto bppi = ports_to_wire_up.begin();
             bppi != ports_to_wire_up.end();
             ++bppi)
        {
            Blueprint::Entity & be = blueprint.entities.at(bppi->entity_id);

            auto & bpp = be.ports.at(bppi->port_num);

            for (auto const & c : bppi->circuits)
            {
                CircuitId cid = c.first;
                WireColor w = c.second;

                if (wired_ports.count(std::make_tuple(bppi->entity_id, bppi->port_num, cid)))
                {
                    continue;
                }

                bool wired_successfully = false;
                if (network_membership.count(cid) != 0)
                {
                    BPPortInfo const * closest = nullptr;
                    double closest_distance = std::numeric_limits<double>::max();
                    for (BPPortInfo const & bpi : network_membership.at(cid))
                    {
                        Blueprint::Entity const & other = blueprint.entities.at(bpi.entity_id);
                        double const distance = Blueprint::distance_between_entities(
                            other.position->x, other.position->y,
                            be.position->x, be.position->y);
                        if (!closest || closest_distance > distance)
                        {
                            closest = &bpi;
                            closest_distance = distance;
                        }
                    }
                    assert(closest);

                    Blueprint::Entity const & other = blueprint.entities.at(closest->entity_id);
                    if (!Blueprint::close_enough_for_direct_connection(
                            other.position->x, other.position->y,
                            be.position->x, be.position->y))
                    {
                        unwired_ports.emplace(bppi->entity_id, bppi->port_num, cid);

                        if (take_desperate_measures)
                        {
                            /* Find a free location where an empty port would be helpful. */
                            int const x_start = other.position->x;
                            int const y_start = other.position->y;

                            int best_new_x = x_start;
                            int best_new_y = y_start;
                            double best_new_distance = Blueprint::distance_between_entities(
                                best_new_x, best_new_y, be.position->x, be.position->y);

                            for (int x_offset = -10; x_offset <= 10; ++x_offset)
                            {
                                for (int y_offset = -10; y_offset <= 10; ++y_offset)
                                {
                                    int const x = x_start + x_offset;
                                    int const y = y_start + y_offset;
                                    if (!Blueprint::close_enough_for_direct_connection(
                                            x, y, other.position->x, other.position->y))
                                    {
                                        continue;
                                    }
                                    if (occupied_locations.count(std::make_pair(x, y)))
                                    {
                                        continue;
                                    }
                                    double const distance = Blueprint::distance_between_entities(
                                        x, y, be.position->x, be.position->y);
                                    if (best_new_distance > distance)
                                    {
                                        best_new_x = x;
                                        best_new_y = y;
                                        best_new_distance = distance;
                                    }
                                }
                            }

                            assert(best_new_x != x_start || best_new_y != y_start);

                            /* Add a constant combinator with no filters at that spot. */
                            Blueprint::Entity new_be;
                            new_be.id = blueprint.entities.size() + 1;
                            new_be.name = Signal::constant_combinator;
                            new_be.control_behavior = Blueprint::Entity::Filters();
                            new_be.position = {
                                static_cast<double>(best_new_x),
                                static_cast<double>(best_new_y),
                            };
                            new_be.direction = 1;
                            new_be.ports[1] = Blueprint::Entity::Port();
                            blueprint.entities[new_be.id] = new_be;
                            occupied_locations.emplace(best_new_x, best_new_y);

                            BPPortInfo new_bppi;
                            new_bppi.entity_id = new_be.id;
                            new_bppi.port_num = 1;
                            new_bppi.port = &new_be.ports.at(1);
                            new_bppi.circuits.emplace_back(cid, w);
                            ports_to_wire_up.push_back(new_bppi);
                            progress = true;
                            take_desperate_measures = false;
                        }
                    }
                    else
                    {
                        wired_successfully = true;

                        assert(w == Wire::red || w == Wire::green);
                        auto & bpp_wires = w == Wire::red ? bpp.red : bpp.green;
                        auto & hub_wires = w == Wire::red ? closest->port->red : closest->port->green;

                        bpp_wires.emplace_back(closest->entity_id, closest->port_num);
                        hub_wires.emplace_back(be.id, bppi->port_num);
                    }
                }
                if (wired_successfully || network_membership.count(cid) == 0)
                {
                    network_membership[cid].push_back({be.id, bppi->port_num, &bpp});
                    wired_ports.emplace(bppi->entity_id, bppi->port_num, cid);
                    unwired_ports.erase(std::make_tuple(bppi->entity_id, bppi->port_num, cid));
                    progress = true;
                    take_desperate_measures = false;
                }
            }
        }
        if (!progress)
        {
            if (take_desperate_measures)
            {
                std::ostringstream err;
                err << "The Emperor is displeased by your apparent lack of progress.\n";
                err << "There are " << unwired_ports.size() << " unwired ports.\n";
                err << "There are " << wired_ports.size() << " wired ports.\n";
                throw std::runtime_error(err.str());
            }
            take_desperate_measures = true;
        }
    } while (unwired_ports.size() > 0);

    /* Wire up the labeled interface combinators. */
    for (std::pair<int, Port const *> id_and_port : interface_id_to_port_map)
    {
        Blueprint::Entity & be = blueprint.entities.at(id_and_port.first);
        Port const * port = id_and_port.second;
        assert(port);

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
    }

    return blueprint.to_blueprint_string();
}
