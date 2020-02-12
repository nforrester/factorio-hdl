#include "Factorio.h"
#include "entities/ArithmeticCombinator.h"
#include "entities/DeciderCombinator.h"
#include "entities/ConstantCombinator.h"
#include "entities/SmallLamp.h"
#include "entities/Counter.h"
#include "entities/Hysteresis.h"
#include "blueprint/Blueprint.h"
#include "util.h"
#include "src/debug.h"

#include <sstream>
#include <unistd.h>

int main(int argc, char ** argv)
{
    assert(argc == 2);

    info(0) << json::parse(blueprint_string_to_raw_json(argv[1])).dump(4) << "\n\n\n";
    info(0).flush();

    Blueprint::Blueprint blueprint(argv[1]);

    Factorio fac;

    std::map<int, std::vector<Port*>> ports_for_entities;
    for (auto const & ie : blueprint.entities)
    {
        Blueprint::Entity const & e = ie.second;
        if (e.name == Signal::constant_combinator)
        {
            auto & c = fac.new_entity<ConstantCombinator>("constant " + std::to_string(e.id) + " > ");
            for (auto const & f : std::get<Blueprint::Entity::Filters>(
                                      *e.control_behavior->spec).filters)
            {
                c.constants.add(f.signal->name, f.count);
            }
            ports_for_entities[e.id].push_back(&c.port("out"));
        }
        else if (e.name == Signal::decider_combinator)
        {
            auto const & dc = std::get<Blueprint::Entity::DeciderConditions>(
                *e.control_behavior->spec);
            DeciderCombinator * d;
            if (dc.rhs_signal.has_value())
            {
                d = &fac.new_entity<DeciderCombinator>(
                    "decider " + std::to_string(e.id) + " > ",
                    dc.lhs->name,
                    dc.op,
                    dc.rhs_signal->name,
                    dc.out->name,
                    dc.copy_count_from_input);
            }
            else
            {
                assert(dc.rhs_const.has_value());
                d = &fac.new_entity<DeciderCombinator>(
                    "decider " + std::to_string(e.id) + " > ",
                    dc.lhs->name,
                    dc.op,
                    *dc.rhs_const,
                    dc.out->name,
                    dc.copy_count_from_input);
            }
            ports_for_entities[e.id].push_back(&d->port("in"));
            ports_for_entities[e.id].push_back(&d->port("out"));
        }
        else if (e.name == Signal::arithmetic_combinator)
        {
            auto const & ac = std::get<Blueprint::Entity::ArithmeticConditions>(
                *e.control_behavior->spec);
            ArithmeticCombinator * a;
            if (ac.rhs_signal.has_value())
            {
                a = &fac.new_entity<ArithmeticCombinator>(
                    "arithmetic " + std::to_string(e.id) + " > ",
                    ac.lhs->name,
                    ac.op,
                    ac.rhs_signal->name,
                    ac.out->name);
            }
            else
            {
                assert(ac.rhs_const.has_value());
                a = &fac.new_entity<ArithmeticCombinator>(
                    "arithmetic " + std::to_string(e.id) + " > ",
                    ac.lhs->name,
                    ac.op,
                    *ac.rhs_const,
                    ac.out->name);
            }
            ports_for_entities[e.id].push_back(&a->port("in"));
            ports_for_entities[e.id].push_back(&a->port("out"));
        }
        else if (e.name == Signal::small_lamp)
        {
            auto const & cc = std::get<Blueprint::Entity::CircuitCondition>(
                *e.control_behavior->spec);
            SmallLamp * l;
            if (cc.rhs_signal.has_value())
            {
                l = &fac.new_entity<SmallLamp>(
                    "lamp " + std::to_string(e.id) + " > ",
                    cc.lhs->name,
                    cc.op,
                    cc.rhs_signal->name,
                    e.control_behavior->use_colors);
            }
            else
            {
                assert(cc.rhs_const.has_value());
                l = &fac.new_entity<SmallLamp>(
                    "lamp " + std::to_string(e.id) + " > ",
                    cc.lhs->name,
                    cc.op,
                    *cc.rhs_const,
                    e.control_behavior->use_colors);
            }
            ports_for_entities[e.id].push_back(&l->port("in"));
        }
        else if (e.name == Signal::iron_chest) // TODO need a more disciplined approach?
        {
            auto & c = fac.new_entity<ConstantCombinator>("iron_chest > ");
            ports_for_entities[e.id].push_back(&c.port("out"));
        }
        else if (e.name == Signal::medium_electric_pole) // TODO need a more disciplined approach?
        {
            auto & c = fac.new_entity<ConstantCombinator>("medium_electric_pole > ");
            ports_for_entities[e.id].push_back(&c.port("out"));
        }
        else
        {
            throw std::runtime_error(FILE_LINE + ": " + get_signal_name_lower_case(e.name));
        }
    }

    for (auto const & ie : blueprint.entities)
    {
        Blueprint::Entity const & e = ie.second;
        for (auto const & ip : e.ports)
        {
            int i = ip.first;
            Blueprint::Entity::Port const & p = ip.second;
            for (Blueprint::Entity::Port::Wire const & w : p.green)
            {
                fac.connect(Wire::green,
                            *ports_for_entities.at(e.id).at(i - 1),
                            *ports_for_entities.at(w.entity_id).at(w.port_num - 1));
            }
            for (Blueprint::Entity::Port::Wire const & w : p.red)
            {
                fac.connect(Wire::red,
                            *ports_for_entities.at(e.id).at(i - 1),
                            *ports_for_entities.at(w.entity_id).at(w.port_num - 1));
            }
        }
    }

    for (auto & eps : ports_for_entities)
    {
        for (Port * p : eps.second)
        {
            fac.lock(Wire::green, *p);
            fac.lock(Wire::red, *p);
        }
    }

    fac.build();

    info(0) << "Begin\n";

    bool stable = false;
    for (int i = 0; true; ++i)
    {
        for (auto const & eps : ports_for_entities)
        {
            std::ostringstream entity_label;
            entity_label << "E" << eps.first;
            std::string entity_space;
            entity_space.insert(0, entity_label.str().size(), ' ');

            info(0) << entity_label.str() << " (green): ";
            for (Port const * p : eps.second)
            {
                info(0) << fac.read(*p, Wire::green) << " ";
            }
            info(0) << "\n";
            info(0) << entity_space << "   (red): ";
            for (Port const * p : eps.second)
            {
                info(0) << fac.read(*p, Wire::red) << " ";
            }
            info(0) << "\n";
        }

        if (i == 1000 || stable)
        {
            break;
        }

        stable = fac.tick();
        info(0) << "\nTick.\n";
    }

    info(0) << "\n";
    if (stable)
    {
        info(0) << "Stable.";
    }
    else
    {
        info(0) << "Did not stabilize before timeout.";
    }
    info(0) << "\n";

    return 0;
}
