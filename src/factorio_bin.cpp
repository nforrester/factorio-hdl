#include "Factorio.h"
#include "entities/ArithmeticCombinator.h"
#include "entities/DeciderCombinator.h"
#include "entities/ConstantCombinator.h"
#include "entities/Machine.h"
#include "entities/Counter.h"
#include "entities/Hysteresis.h"
#include "blueprint/Blueprint.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

// TODO DEDUP
#define FILE_LINE std::string(__FILE__ ":" + std::to_string(__LINE__))

namespace {
    // TODO make more generally useful
    std::map<std::string, SignalId> const sids = {
        {"signal-A", Signal::letter_a},
        {"signal-B", Signal::letter_b},
        {"signal-C", Signal::letter_c},
        {"signal-D", Signal::letter_d},
        {"signal-E", Signal::letter_e},
        {"signal-F", Signal::letter_f},
        {"signal-G", Signal::letter_g},
        {"signal-H", Signal::letter_h},
        {"signal-I", Signal::letter_i},
        {"signal-J", Signal::letter_j},
        {"signal-K", Signal::letter_k},
        {"signal-L", Signal::letter_l},
        {"signal-M", Signal::letter_m},
        {"signal-N", Signal::letter_n},
        {"signal-O", Signal::letter_o},
        {"signal-P", Signal::letter_p},
        {"signal-Q", Signal::letter_q},
        {"signal-R", Signal::letter_r},
        {"signal-S", Signal::letter_s},
        {"signal-T", Signal::letter_t},
        {"signal-U", Signal::letter_u},
        {"signal-V", Signal::letter_v},
        {"signal-W", Signal::letter_w},
        {"signal-X", Signal::letter_x},
        {"signal-Y", Signal::letter_y},
        {"signal-Z", Signal::letter_z},
    };

    SignalId get_sid(std::string const & name)
    {
        try
        {
            return sids.at(name);
        }
        catch (std::out_of_range & e)
        {
            throw std::runtime_error(FILE_LINE + ": " + name);
        }
    }
}

int main(int argc, char ** argv)
{
    assert(argc == 2);

    std::cout << json::parse(blueprint_string_to_raw_json(argv[1])).dump(4) << "\n\n\n";

    Blueprint blueprint(argv[1]);

    Factorio fac;

    std::map<int, std::vector<Port*>> ports_for_entities;
    for (auto const & ie : blueprint.entities)
    {
        Blueprint::Entity const & e = ie.second;
        if (e.name == "constant-combinator")
        {
            auto & c = fac.new_entity<ConstantCombinator>();
            for (auto const & f : std::get<Blueprint::Entity::ControlBehavior::Filters>(
                                      *e.control_behavior->behavior).filters)
            {
                c.constants.add(get_sid(f.signal->name), f.count);
            }
            ports_for_entities[e.id].push_back(&c.port);
        }
        else if (e.name == "decider-combinator")
        {
            auto const & dc = std::get<Blueprint::Entity::ControlBehavior::DeciderConditions>(
                *e.control_behavior->behavior);
            DeciderCombinator * d;
            if (dc.rhs_signal.has_value())
            {
                d = &fac.new_entity<DeciderCombinator>(
                    get_sid(dc.lhs->name),
                    dc.op,
                    get_sid(dc.rhs_signal->name),
                    get_sid(dc.out->name),
                    dc.copy_count_from_input);
            }
            else
            {
                assert(dc.rhs_const.has_value());
                d = &fac.new_entity<DeciderCombinator>(
                    get_sid(dc.lhs->name),
                    dc.op,
                    *dc.rhs_const,
                    get_sid(dc.out->name),
                    dc.copy_count_from_input);
            }
            ports_for_entities[e.id].push_back(&d->in_port);
            ports_for_entities[e.id].push_back(&d->out_port);
        }
        else if (e.name == "arithmetic-combinator")
        {
            auto const & ac = std::get<Blueprint::Entity::ControlBehavior::ArithmeticConditions>(
                *e.control_behavior->behavior);
            ArithmeticCombinator * a;
            if (ac.rhs_signal.has_value())
            {
                a = &fac.new_entity<ArithmeticCombinator>(
                    get_sid(ac.lhs->name),
                    ac.op,
                    get_sid(ac.rhs_signal->name),
                    get_sid(ac.out->name));
            }
            else
            {
                assert(ac.rhs_const.has_value());
                a = &fac.new_entity<ArithmeticCombinator>(
                    get_sid(ac.lhs->name),
                    ac.op,
                    *ac.rhs_const,
                    get_sid(ac.out->name));
            }
            ports_for_entities[e.id].push_back(&a->in_port);
            ports_for_entities[e.id].push_back(&a->out_port);
        }
        else if (e.name == "iron-chest") // TODO need a more disciplined approach?
        {
            auto & c = fac.new_entity<ConstantCombinator>();
            ports_for_entities[e.id].push_back(&c.port);
        }
        else
        {
            throw std::runtime_error(FILE_LINE + ": " + e.name);
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

    std::cout << "Begin\n";

    bool stable = false;
    for (int i = 0; true; ++i)
    {
        for (auto const & eps : ports_for_entities)
        {
            std::ostringstream entity_label;
            entity_label << "E" << eps.first;
            std::string entity_space;
            entity_space.insert(0, entity_label.str().size(), ' ');

            std::cout << entity_label.str() << " (green): ";
            for (Port const * p : eps.second)
            {
                std::cout << fac.read(*p, Wire::green) << " ";
            }
            std::cout << "\n";
            std::cout << entity_space << "   (red): ";
            for (Port const * p : eps.second)
            {
                std::cout << fac.read(*p, Wire::red) << " ";
            }
            std::cout << "\n";
        }

        if (i == 1000 || stable)
        {
            break;
        }

        stable = fac.tick();
        std::cout << "\nTick.\n";
    }

    std::cout << "\n";
    if (stable)
    {
        std::cout << "Stable.";
    }
    else
    {
        std::cout << "Did not stabilize before timeout.";
    }
    std::cout << "\n";

    return 0;
}
