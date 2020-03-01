#include "Blueprint.h"
#include "Base64.h"
#include "zlib_easy.h"
#include "JsonDigester.h"
#include "src/util.h"
#include "src/debug.h"

#include <algorithm>

namespace {
    std::map<std::string, SignalId> const name_to_signal_id_map = {
        {"wooden-chest",                     Signal::wooden_chest},
        {"iron-chest",                       Signal::iron_chest},
        {"steel-chest",                      Signal::steel_chest},
        {"storage-tank",                     Signal::storage_tank},
        {"transport-belt",                   Signal::transport_belt},
        {"fast-transport-belt",              Signal::fast_transport_belt},
        {"express-transport-belt",           Signal::express_transport_belt},
        {"underground-belt",                 Signal::underground_belt},
        {"fast-underground-belt",            Signal::fast_underground_belt},
        {"express-underground-belt",         Signal::express_underground_belt},
        {"splitter",                         Signal::splitter},
        {"fast-splitter",                    Signal::fast_splitter},
        {"express-splitter",                 Signal::express_splitter},
        {"burner-inserter",                  Signal::burner_inserter},
        {"inserter",                         Signal::inserter},
        {"long-handed-inserter",             Signal::long_handed_inserter},
        {"fast-inserter",                    Signal::fast_inserter},
        {"filter-inserter",                  Signal::filter_inserter},
        {"stack-inserter",                   Signal::stack_inserter},
        {"stack-filter-inserter",            Signal::stack_filter_inserter},
        {"small-electric-pole",              Signal::small_electric_pole},
        {"medium-electric-pole",             Signal::medium_electric_pole},
        {"big-electric-pole",                Signal::big_electric_pole},
        {"substation",                       Signal::substation},
        {"pipe",                             Signal::pipe},
        {"pipe-to-ground",                   Signal::pipe_to_ground},
        {"pump",                             Signal::pump},
        {"rail",                             Signal::rail},
        {"train-stop",                       Signal::train_stop},
        {"rail-signal",                      Signal::rail_signal},
        {"rail-chain-signal",                Signal::rail_chain_signal},
        {"locomotive",                       Signal::locomotive},
        {"cargo-wagon",                      Signal::cargo_wagon},
        {"fluid-wagon",                      Signal::fluid_wagon},
        {"artillery-wagon",                  Signal::artillery_wagon},
        {"car",                              Signal::car},
        {"tank",                             Signal::tank},
        {"logistic-robot",                   Signal::logistic_robot},
        {"construction-robot",               Signal::construction_robot},
        {"logistic-chest-active-provider",   Signal::logistic_chest_active_provider},
        {"logistic-chest-passive-provider",  Signal::logistic_chest_passive_provider},
        {"logistic-chest-storage",           Signal::logistic_chest_storage},
        {"logistic-chest-buffer",            Signal::logistic_chest_buffer},
        {"logistic-chest-requester",         Signal::logistic_chest_requester},
        {"roboport",                         Signal::roboport},
        {"small-lamp",                       Signal::small_lamp},
        {"red-wire",                         Signal::red_wire},
        {"green-wire",                       Signal::green_wire},
        {"arithmetic-combinator",            Signal::arithmetic_combinator},
        {"decider-combinator",               Signal::decider_combinator},
        {"constant-combinator",              Signal::constant_combinator},
        {"power-switch",                     Signal::power_switch},
        {"programmable-speaker",             Signal::programmable_speaker},
        {"stone-brick",                      Signal::stone_brick},
        {"concrete",                         Signal::concrete},
        {"hazard-concrete",                  Signal::hazard_concrete},
        {"refined-concrete",                 Signal::refined_concrete},
        {"refined-hazard-concrete",          Signal::refined_hazard_concrete},
        {"landfill",                         Signal::landfill},
        {"cliff-explosives",                 Signal::cliff_explosives},
        {"repair-pack",                      Signal::repair_pack},
        {"blueprint",                        Signal::blueprint},
        {"deconstruction-planner",           Signal::deconstruction_planner},
        {"upgrade-planner",                  Signal::upgrade_planner},
        {"blueprint-book",                   Signal::blueprint_book},
        {"boiler",                           Signal::boiler},
        {"steam-engine",                     Signal::steam_engine},
        {"steam-turbine",                    Signal::steam_turbine},
        {"solar-panel",                      Signal::solar_panel},
        {"accumulator",                      Signal::accumulator},
        {"nuclear-reactor",                  Signal::nuclear_reactor},
        {"heat-exchanger",                   Signal::heat_exchanger},
        {"heat-pipe",                        Signal::heat_pipe},
        {"burner-mining-drill",              Signal::burner_mining_drill},
        {"electric-mining-drill",            Signal::electric_mining_drill},
        {"offshore-pump",                    Signal::offshore_pump},
        {"pumpjack",                         Signal::pumpjack},
        {"stone-furnace",                    Signal::stone_furnace},
        {"steel-furnace",                    Signal::steel_furnace},
        {"electric-furnace",                 Signal::electric_furnace},
        {"assembling-machine-1",             Signal::assembling_machine_1},
        {"assembling-machine-2",             Signal::assembling_machine_2},
        {"assembling-machine-3",             Signal::assembling_machine_3},
        {"oil-refinery",                     Signal::oil_refinery},
        {"chemical-plant",                   Signal::chemical_plant},
        {"centrifuge",                       Signal::centrifuge},
        {"lab",                              Signal::lab},
        {"beacon",                           Signal::beacon},
        {"speed-module",                     Signal::speed_module},
        {"speed-module-2",                   Signal::speed_module_2},
        {"speed-module-3",                   Signal::speed_module_3},
        {"effectivity-module",               Signal::effectivity_module},
        {"effectivity-module-2",             Signal::effectivity_module_2},
        {"effectivity-module-3",             Signal::effectivity_module_3},
        {"productivity-module",              Signal::productivity_module},
        {"productivity-module-2",            Signal::productivity_module_2},
        {"productivity-module-3",            Signal::productivity_module_3},
        {"wood",                             Signal::wood},
        {"coal",                             Signal::coal},
        {"stone",                            Signal::stone},
        {"iron-ore",                         Signal::iron_ore},
        {"copper-ore",                       Signal::copper_ore},
        {"uranium-ore",                      Signal::uranium_ore},
        {"raw-fish",                         Signal::raw_fish},
        {"iron-plate",                       Signal::iron_plate},
        {"copper-plate",                     Signal::copper_plate},
        {"solid-fuel",                       Signal::solid_fuel},
        {"steel-plate",                      Signal::steel_plate},
        {"plastic-bar",                      Signal::plastic_bar},
        {"sulfur",                           Signal::sulfur},
        {"battery",                          Signal::battery},
        {"explosives",                       Signal::explosives},
        {"crude-oil-barrel",                 Signal::crude_oil_barrel},
        {"heavy-oil-barrel",                 Signal::heavy_oil_barrel},
        {"light-oil-barrel",                 Signal::light_oil_barrel},
        {"lubricant-barrel",                 Signal::lubricant_barrel},
        {"petroleum-gas-barrel",             Signal::petroleum_gas_barrel},
        {"sulfuric-acid-barrel",             Signal::sulfuric_acid_barrel},
        {"water-barrel",                     Signal::water_barrel},
        {"copper-cable",                     Signal::copper_cable},
        {"iron-stick",                       Signal::iron_stick},
        {"iron-gear-wheel",                  Signal::iron_gear_wheel},
        {"empty-barrel",                     Signal::empty_barrel},
        {"electronic-circuit",               Signal::electronic_circuit},
        {"advanced-circuit",                 Signal::advanced_circuit},
        {"processing-unit",                  Signal::processing_unit},
        {"engine-unit",                      Signal::engine_unit},
        {"electric-engine-unit",             Signal::electric_engine_unit},
        {"flying-robot-frame",               Signal::flying_robot_frame},
        {"satellite",                        Signal::satellite},
        {"rocket-part",                      Signal::rocket_part},
        {"rocket-control-unit",              Signal::rocket_control_unit},
        {"low-density-structure",            Signal::low_density_structure},
        {"rocket-fuel",                      Signal::rocket_fuel},
        {"nuclear-fuel",                     Signal::nuclear_fuel},
        {"uranium-235",                      Signal::uranium_235},
        {"uranium-238",                      Signal::uranium_238},
        {"uranium-fuel-cell",                Signal::uranium_fuel_cell},
        {"used-up-uranium-fuel-cell",        Signal::used_up_uranium_fuel_cell},
        {"automation-science-pack",          Signal::automation_science_pack},
        {"logistic-science-pack",            Signal::logistic_science_pack},
        {"military-science-pack",            Signal::military_science_pack},
        {"chemical-science-pack",            Signal::chemical_science_pack},
        {"production-science-pack",          Signal::production_science_pack},
        {"utility-science-pack",             Signal::utility_science_pack},
        {"space-science-pack",               Signal::space_science_pack},
        {"pistol",                           Signal::pistol},
        {"submachine-gun",                   Signal::submachine_gun},
        {"shotgun",                          Signal::shotgun},
        {"combat-shotgun",                   Signal::combat_shotgun},
        {"rocket-launcher",                  Signal::rocket_launcher},
        {"flamethrower",                     Signal::flamethrower},
        {"land-mine",                        Signal::land_mine},
        {"firearm-magazine",                 Signal::firearm_magazine},
        {"piercing-rounds-magazine",         Signal::piercing_rounds_magazine},
        {"uranium-rounds-magazine",          Signal::uranium_rounds_magazine},
        {"shotgun-shell",                    Signal::shotgun_shell},
        {"piercing-shotgun-shell",           Signal::piercing_shotgun_shell},
        {"cannon-shell",                     Signal::cannon_shell},
        {"explosive-cannon-shell",           Signal::explosive_cannon_shell},
        {"uranium-cannon-shell",             Signal::uranium_cannon_shell},
        {"explosive-uranium-cannon-shell",   Signal::explosive_uranium_cannon_shell},
        {"artillery-shell",                  Signal::artillery_shell},
        {"rocket",                           Signal::rocket},
        {"explosive-rocket",                 Signal::explosive_rocket},
        {"atomic-bomb",                      Signal::atomic_bomb},
        {"flamethrower-ammo",                Signal::flamethrower_ammo},
        {"grenade",                          Signal::grenade},
        {"cluster-grenade",                  Signal::cluster_grenade},
        {"poison-capsule",                   Signal::poison_capsule},
        {"slowdown-capsule",                 Signal::slowdown_capsule},
        {"defender-capsule",                 Signal::defender_capsule},
        {"distractor-capsule",               Signal::distractor_capsule},
        {"destroyer-capsule",                Signal::destroyer_capsule},
        {"discharge-defense-remote",         Signal::discharge_defense_remote},
        {"artillery-targeting-remote",       Signal::artillery_targeting_remote},
        {"light-armor",                      Signal::light_armor},
        {"heavy-armor",                      Signal::heavy_armor},
        {"modular-armor",                    Signal::modular_armor},
        {"power-armor",                      Signal::power_armor},
        {"power-armor-mk2",                  Signal::power_armor_mk2},
        {"solar-panel-equipment",            Signal::solar_panel_equipment},
        {"fusion-reactor-equipment",         Signal::fusion_reactor_equipment},
        {"energy-shield-equipment",          Signal::energy_shield_equipment},
        {"energy-shield-mk2-equipment",      Signal::energy_shield_mk2_equipment},
        {"battery-equipment",                Signal::battery_equipment},
        {"battery-mk2-equipment",            Signal::battery_mk2_equipment},
        {"personal-laser-defense-equipment", Signal::personal_laser_defense_equipment},
        {"discharge-defense-equipment",      Signal::discharge_defense_equipment},
        {"belt-immunity-equipment",          Signal::belt_immunity_equipment},
        {"exoskeleton-equipment",            Signal::exoskeleton_equipment},
        {"personal-roboport-equipment",      Signal::personal_roboport_equipment},
        {"personal-roboport-mk2-equipment",  Signal::personal_roboport_mk2_equipment},
        {"night-vision-equipment",           Signal::night_vision_equipment},
        {"stone-wall",                       Signal::stone_wall},
        {"gate",                             Signal::gate},
        {"gun-turret",                       Signal::gun_turret},
        {"laser-turret",                     Signal::laser_turret},
        {"flamethrower-turret",              Signal::flamethrower_turret},
        {"artillery-turret",                 Signal::artillery_turret},
        {"radar",                            Signal::radar},
        {"rocket-silo",                      Signal::rocket_silo},
        {"crude-oil",                        Signal::crude_oil},
        {"heavy-oil",                        Signal::heavy_oil},
        {"light-oil",                        Signal::light_oil},
        {"lubricant",                        Signal::lubricant},
        {"petroleum-gas",                    Signal::petroleum_gas},
        {"sulfuric-acid",                    Signal::sulfuric_acid},
        {"water",                            Signal::water},
        {"steam",                            Signal::steam},
        {"signal-0",                         Signal::signal_0},
        {"signal-1",                         Signal::signal_1},
        {"signal-2",                         Signal::signal_2},
        {"signal-3",                         Signal::signal_3},
        {"signal-4",                         Signal::signal_4},
        {"signal-5",                         Signal::signal_5},
        {"signal-6",                         Signal::signal_6},
        {"signal-7",                         Signal::signal_7},
        {"signal-8",                         Signal::signal_8},
        {"signal-9",                         Signal::signal_9},
        {"signal-A",                         Signal::signal_A},
        {"signal-B",                         Signal::signal_B},
        {"signal-C",                         Signal::signal_C},
        {"signal-D",                         Signal::signal_D},
        {"signal-E",                         Signal::signal_E},
        {"signal-F",                         Signal::signal_F},
        {"signal-G",                         Signal::signal_G},
        {"signal-H",                         Signal::signal_H},
        {"signal-I",                         Signal::signal_I},
        {"signal-J",                         Signal::signal_J},
        {"signal-K",                         Signal::signal_K},
        {"signal-L",                         Signal::signal_L},
        {"signal-M",                         Signal::signal_M},
        {"signal-N",                         Signal::signal_N},
        {"signal-O",                         Signal::signal_O},
        {"signal-P",                         Signal::signal_P},
        {"signal-Q",                         Signal::signal_Q},
        {"signal-R",                         Signal::signal_R},
        {"signal-S",                         Signal::signal_S},
        {"signal-T",                         Signal::signal_T},
        {"signal-U",                         Signal::signal_U},
        {"signal-V",                         Signal::signal_V},
        {"signal-W",                         Signal::signal_W},
        {"signal-X",                         Signal::signal_X},
        {"signal-Y",                         Signal::signal_Y},
        {"signal-Z",                         Signal::signal_Z},
        {"signal-red",                       Signal::signal_red},
        {"signal-green",                     Signal::signal_green},
        {"signal-blue",                      Signal::signal_blue},
        {"signal-yellow",                    Signal::signal_yellow},
        {"signal-pink",                      Signal::signal_pink},
        {"signal-cyan",                      Signal::signal_cyan},
        {"signal-white",                     Signal::signal_white},
        {"signal-grey",                      Signal::signal_grey},
        {"signal-black",                     Signal::signal_black},
        {"signal-check",                     Signal::signal_check},
        {"signal-info",                      Signal::signal_info},
        {"signal-dot",                       Signal::signal_dot},
        {"signal-everything",                LogicSignal::signal_everything},
        {"signal-anything",                  LogicSignal::signal_anything},
        {"signal-each",                      LogicSignal::signal_each},
    };

    SignalId get_signal_id(std::string const & name)
    {
        try
        {
            return name_to_signal_id_map.at(name);
        }
        catch (std::out_of_range & e)
        {
            throw std::runtime_error(FILE_LINE + ": " + name);
        }
    }

    std::string get_signal_name(SignalId id)
    {
        // TODO don't remake this every time
        std::map<SignalId, std::string> signal_id_to_name_map;
        for (auto const & kv : name_to_signal_id_map)
        {
            assert(signal_id_to_name_map.count(kv.second) == 0);
            signal_id_to_name_map[kv.second] = kv.first;
        }

        try
        {
            return signal_id_to_name_map.at(id);
        }
        catch (std::out_of_range & e)
        {
            throw std::runtime_error(FILE_LINE + ": " + std::to_string(id));
        }
    }

    std::map<std::string, DeciderCombinator::Op> decider_op_map =
    {
        {"<", DeciderCombinator::Op::LT},
        {">", DeciderCombinator::Op::GT},
        {"≤", DeciderCombinator::Op::LE},
        {"≥", DeciderCombinator::Op::GE},
        {"=", DeciderCombinator::Op::EQ},
        {"≠", DeciderCombinator::Op::NE},
    };

    DeciderCombinator::Op get_decider_op(std::string const & name)
    {
        try
        {
            return decider_op_map.at(name);
        }
        catch (std::out_of_range & e)
        {
            throw std::runtime_error(FILE_LINE + ": " + name);
        }
    }

    std::string get_decider_op_name(DeciderCombinator::Op op)
    {
        // TODO don't remake this every time
        std::map<DeciderCombinator::Op, std::string> rev_map;
        for (auto const & kv : decider_op_map)
        {
            assert(rev_map.count(kv.second) == 0);
            rev_map[kv.second] = kv.first;
        }

        try
        {
            return rev_map.at(op);
        }
        catch (std::out_of_range & e)
        {
            throw std::runtime_error(FILE_LINE + ": " + std::to_string(static_cast<int>(op)));
        }
    }

    std::map<std::string, ArithmeticCombinator::Op> arithmetic_op_map =
    {
        {"+", ArithmeticCombinator::Op::ADD},
        {"-", ArithmeticCombinator::Op::SUB},
        {"*", ArithmeticCombinator::Op::MUL},
        {"/", ArithmeticCombinator::Op::DIV},
        {"%", ArithmeticCombinator::Op::MOD},
        {"^", ArithmeticCombinator::Op::POW},
        {"<<", ArithmeticCombinator::Op::LSH},
        {">>", ArithmeticCombinator::Op::RSH},
        {"AND", ArithmeticCombinator::Op::AND},
        {"OR", ArithmeticCombinator::Op::OR},
        {"XOR", ArithmeticCombinator::Op::XOR},
    };

    ArithmeticCombinator::Op get_arithmetic_op(std::string const & name)
    {
        try
        {
            return arithmetic_op_map.at(name);
        }
        catch (std::out_of_range & e)
        {
            throw std::runtime_error(FILE_LINE + ": " + name);
        }
    }

    std::string get_arithmetic_op_name(ArithmeticCombinator::Op op)
    {
        // TODO don't remake this every time
        std::map<ArithmeticCombinator::Op, std::string> rev_map;
        for (auto const & kv : arithmetic_op_map)
        {
            assert(rev_map.count(kv.second) == 0);
            rev_map[kv.second] = kv.first;
        }

        try
        {
            return rev_map.at(op);
        }
        catch (std::out_of_range & e)
        {
            throw std::runtime_error(FILE_LINE + ": " + std::to_string(static_cast<int>(op)));
        }
    }
}

SignalId get_signal_id_from_lower_case(std::string const & name)
{
    // TODO don't remake this every time
    std::map<std::string, SignalId> lower_case_map;
    for (auto const & kv : name_to_signal_id_map)
    {
        std::string lower_key = kv.first;
        std::transform(
            lower_key.begin(),
            lower_key.end(),
            lower_key.begin(),
            [](unsigned char c){ return std::tolower(c); });
        assert(lower_case_map.count(lower_key) == 0);
        lower_case_map[lower_key] = kv.second;
    }

    try
    {
        return lower_case_map.at(name);
    }
    catch (std::out_of_range & e)
    {
        throw std::runtime_error(FILE_LINE + ": " + name);
    }
}

std::string get_signal_name_lower_case(SignalId id)
{
    std::string lower_name = get_signal_name(id);
    std::transform(
        lower_name.begin(),
        lower_name.end(),
        lower_name.begin(),
        [](unsigned char c){ return std::tolower(c); });

    return lower_name;
}

std::string blueprint_string_to_raw_json(std::string const & blueprint_string)
{
    assert(blueprint_string.size() > 0);
    assert(blueprint_string.at(0) == '0'); // Version byte
    std::string const base64_encoded = blueprint_string.substr(1);

    std::string zlib_compressed;
    assert("" == macaron::Base64::Decode(base64_encoded, zlib_compressed));

    std::string const raw_json = zlib_decompress(zlib_compressed);

    return raw_json;
}

std::string raw_json_to_blueprint_string(std::string const & raw_json)
{
    std::string const zlib_compressed = zlib_compress(raw_json);
    std::string const base64_encoded = macaron::Base64::Encode(zlib_compressed);
    std::string const blueprint_string = "0" + base64_encoded; // Version byte

    return blueprint_string;
}

std::string Blueprint::Blueprint::to_blueprint_string() const
{
    std::string bs = raw_json_to_blueprint_string(to_json().dump());
    Blueprint check(bs); // Check that it can be read back in.
    return bs;
}

Blueprint::Blueprint::Blueprint()
{
    item = "blueprint";
    label = "";
    version = 0;
}

Blueprint::Blueprint::Blueprint(std::string const & blueprint_string)
{
    JsonDigester<Blueprint> d;

    d.require("blueprint", [](json const & j, Blueprint & bp)
    {
        JsonDigester<Blueprint> d;

        d.require("icons", [](json const & j, Blueprint & bp)
        {
            assert(j.is_array());
            for (json const & jj : j)
            {
                bp.icons.emplace_back(jj);
            }
        });

        d.require("entities", [](json const & j, Blueprint & bp)
        {
            assert(j.is_array());
            for (json const & jj : j)
            {
                Entity e(jj);
                assert(bp.entities.emplace(e.id, e).second);
            }
        });

        d.require("item", [](json const & j, Blueprint & bp)
        {
            bp.item = j;
            assert(bp.item == "blueprint");
        });

        d.optional("label", [](json const & j, Blueprint & bp)
        {
            bp.label = j;
        });

        d.require("version", [](json const & j, Blueprint & bp) { bp.version = j; });

        d.digest(j, bp);
    });

    d.digest(json::parse(blueprint_string_to_raw_json(blueprint_string)), *this);
}

json Blueprint::Blueprint::to_json() const
{
    json j;
    for (auto const & x : icons)
    {
        j["blueprint"]["icons"].push_back(x.to_json());
    }
    for (auto const & x : entities)
    {
        j["blueprint"]["entities"].push_back(x.second.to_json());
    }
    j["blueprint"]["item"] = item;
    if (label != "")
    {
        j["blueprint"]["label"] = label;
    }
    j["blueprint"]["version"] = version;
    return j;
}

Blueprint::Icon::Icon(json const & j)
{
    JsonDigester<Icon> d;

    d.require("index", [](json const & j, Icon & i) { i.index = j; });
    d.require("signal", [](json const & j, Icon & i){ i.signal = std::make_optional(j); });

    d.digest(j, *this);

    assert(index > 0);
}

json Blueprint::Icon::to_json() const
{
    assert(signal.has_value());

    json j;
    j["index"] = index;
    j["signal"] = signal->to_json();
    return j;
}

Blueprint::Entity::Entity(json const & j)
{
    JsonDigester<Entity> d;

    d.require("entity_number", [](json const & j, Entity & e){ e.id = j; });

    d.optional("connections", [](json const & j, Entity & e)
    {
        assert(j.is_object());
        for (auto const & [port_string, pj] : j.items())
        {
            int const port_id = std::stoi(port_string);
            assert(port_id > 0);
            assert(e.ports.emplace(port_id, pj).second);
        }
    });

    d.optional("control_behavior", [](json const & j, Entity & e)
    {
        e.control_behavior = std::make_optional(j);
    });

    d.optional("direction", [](json const & j, Entity & e) { e.direction = j; });
    d.require("name", [](json const & j, Entity & e) { e.name = get_signal_id(j); });

    d.optional("position", [](json const & j, Entity & e)
    {
        e.position = std::make_optional(j);
    });

    d.digest(j, *this);

    assert(id > 0);
    assert(direction > 0);
}

json Blueprint::Entity::to_json() const
{
    assert(position.has_value());

    json j;

    j["entity_number"] = id;

    for (auto const & kv : ports)
    {
        j["connections"][std::to_string(kv.first)] = kv.second.to_json();
    }

    if (control_behavior.has_value())
    {
        j["control_behavior"] = control_behavior->to_json();
    }

    if (direction != 1)
    {
        j["direction"] = direction;
    }

    j["name"] = get_signal_name(name);
    j["position"] = position->to_json();

    return j;
}

Blueprint::Entity::Port::Port(json const & j)
{
    JsonDigester<Port> d;

    d.optional("green", [](json const & j, Port & p)
    {
        assert(j.is_array());
        for (json const & jj : j)
        {
            p.green.emplace_back(jj);
        }
    });

    d.optional("red", [](json const & j, Port & p)
    {
        assert(j.is_array());
        for (json const & jj : j)
        {
            p.red.emplace_back(jj);
        }
    });

    d.digest(j, *this);
}

json Blueprint::Entity::Port::to_json() const
{
    json j;

    assert(green.size() != 0 || red.size() != 0);
    for (Wire const & w : green)
    {
        j["green"].push_back(w.to_json());
    }
    for (Wire const & w : red)
    {
        j["red"].push_back(w.to_json());
    }

    return j;
}

Blueprint::Entity::Port::Wire::Wire(json const & j)
{
    JsonDigester<Wire> d;

    d.require("entity_id", [](json const & j, Wire & w){ w.entity_id = j; });
    d.optional("circuit_id", [](json const & j, Wire & w){ w.port_num = j; });

    d.digest(j, *this);

    assert(entity_id > 0);
    assert(port_num > 0);
}

json Blueprint::Entity::Port::Wire::to_json() const
{
    json j;
    j["entity_id"] = entity_id;
    j["circuit_id"] = port_num;
    return j;
}

Blueprint::Entity::ArithmeticConditions::ArithmeticConditions(json const & j)
{
    JsonDigester<ArithmeticConditions> d;

    d.require("first_signal", [](json const & j, ArithmeticConditions & a)
    {
        a.lhs = std::make_optional(j);
    });

    d.optional("second_signal", [](json const & j, ArithmeticConditions & a)
    {
        a.rhs_signal = std::make_optional(j);
    });

    d.optional("constant", [](json const & j, ArithmeticConditions & a)
    {
        a.rhs_const = std::make_optional(j);
    });

    d.require("output_signal", [](json const & j, ArithmeticConditions & a)
    {
        a.out = std::make_optional(j);
    });

    d.require("operation", [](json const & j, ArithmeticConditions & a)
    {
        a.op = get_arithmetic_op(j);
    });

    d.digest(j, *this);

    assert(rhs_signal.has_value() || rhs_const.has_value());
}

json Blueprint::Entity::ArithmeticConditions::to_json() const
{
    assert(lhs.has_value());
    assert(out.has_value());

    json j;
    j["arithmetic_conditions"]["first_signal"] = lhs->to_json();
    if (rhs_signal.has_value())
    {
        j["arithmetic_conditions"]["second_signal"] = rhs_signal->to_json();
    }
    else
    {
        assert(rhs_const.has_value());
        j["arithmetic_conditions"]["constant"] = *rhs_const;
    }
    j["arithmetic_conditions"]["output_signal"] = out->to_json();
    j["arithmetic_conditions"]["operation"] = get_arithmetic_op_name(op);
    return j;
}

Blueprint::Entity::DeciderConditions::DeciderConditions(json const & j)
{
    JsonDigester<DeciderConditions> d;

    d.require("first_signal", [](json const & j, DeciderConditions & a)
    {
        a.lhs = std::make_optional(j);
    });

    d.optional("second_signal", [](json const & j, DeciderConditions & a)
    {
        a.rhs_signal = std::make_optional(j);
    });

    d.optional("constant", [](json const & j, DeciderConditions & a)
    {
        a.rhs_const = std::make_optional(j);
    });

    d.require("output_signal", [](json const & j, DeciderConditions & a)
    {
        a.out = std::make_optional(j);
    });

    d.require("copy_count_from_input", [](json const & j, DeciderConditions & a)
    {
        a.copy_count_from_input = j;
    });

    d.require("comparator", [](json const & j, DeciderConditions & a)
    {
        a.op = get_decider_op(j);
    });

    d.digest(j, *this);

    assert(rhs_signal.has_value() || rhs_const.has_value());
}

json Blueprint::Entity::DeciderConditions::to_json() const
{
    assert(lhs.has_value());
    assert(out.has_value());

    json j;
    j["decider_conditions"]["first_signal"] = lhs->to_json();
    if (rhs_signal.has_value())
    {
        j["decider_conditions"]["second_signal"] = rhs_signal->to_json();
    }
    else
    {
        assert(rhs_const.has_value());
        j["decider_conditions"]["constant"] = *rhs_const;
    }
    j["decider_conditions"]["output_signal"] = out->to_json();
    j["decider_conditions"]["copy_count_from_input"] = copy_count_from_input;
    j["decider_conditions"]["comparator"] = get_decider_op_name(op);
    return j;
}

Blueprint::Entity::CircuitCondition::CircuitCondition(json const & j)
{
    JsonDigester<CircuitCondition> d;

    d.require("first_signal", [](json const & j, CircuitCondition & a)
    {
        a.lhs = std::make_optional(j);
    });

    d.optional("second_signal", [](json const & j, CircuitCondition & a)
    {
        a.rhs_signal = std::make_optional(j);
    });

    d.optional("constant", [](json const & j, CircuitCondition & a)
    {
        a.rhs_const = std::make_optional(j);
    });

    d.require("comparator", [](json const & j, CircuitCondition & a)
    {
        a.op = get_decider_op(j);
    });

    d.digest(j, *this);

    assert(rhs_signal.has_value() || rhs_const.has_value());
}

json Blueprint::Entity::CircuitCondition::to_json() const
{
    assert(lhs.has_value());

    json j;
    j["circuit_condition"]["first_signal"] = lhs->to_json();
    if (rhs_signal.has_value())
    {
        j["circuit_condition"]["second_signal"] = rhs_signal->to_json();
    }
    else
    {
        assert(rhs_const.has_value());
        j["circuit_condition"]["constant"] = *rhs_const;
    }
    j["circuit_condition"]["comparator"] = get_decider_op_name(op);
    return j;
}

Blueprint::Entity::Filters::Filters(json const & j)
{
    assert(j.is_array());

    for (json const & jj : j)
    {
        filters.emplace_back(jj);
    }
}

json Blueprint::Entity::Filters::to_json() const
{
    json j;
    j["filters"] = json::array();
    for (auto const & f : filters)
    {
        j["filters"].push_back(f.to_json());
    }
    return j;
}

Blueprint::Entity::Filters::Filter::Filter(json const & j)
{
    JsonDigester<Filter> d;

    d.require("count", [](json const & j, Filter & f){ f.count = j; });
    d.require("index", [](json const & j, Filter & f){ f.index = j; });
    d.require("signal", [](json const & j, Filter & f){ f.signal = std::make_optional(j); });

    d.digest(j, *this);

    assert(index > 0);
}

json Blueprint::Entity::Filters::Filter::to_json() const
{
    json j;
    j["count"] = count;
    j["index"] = index;
    j["signal"] = signal->to_json();
    return j;
}

Blueprint::Entity::ControlBehavior::ControlBehavior(json const & j)
{
    JsonDigester<ControlBehavior> d;

    d.optional("arithmetic_conditions", [](json const & j, ControlBehavior & b)
    {
        assert(!b.spec.has_value());
        b.spec = std::make_optional(ArithmeticConditions(j));
    });

    d.optional("decider_conditions", [](json const & j, ControlBehavior & b)
    {
        assert(!b.spec.has_value());
        b.spec = std::make_optional(DeciderConditions(j));
    });

    d.optional("circuit_condition", [](json const & j, ControlBehavior & b)
    {
        assert(!b.spec.has_value());
        b.spec = std::make_optional(CircuitCondition(j));
    });

    d.optional("filters", [](json const & j, ControlBehavior & b)
    {
        assert(!b.spec.has_value());
        b.spec = std::make_optional(Filters(j));
    });

    d.optional("use_colors", [](json const & j, ControlBehavior & b)
    {
        b.use_colors = j;
    });

    d.digest(j, *this);

    assert(spec.has_value());
}

json Blueprint::Entity::ControlBehavior::to_json() const
{
    json j = std::visit([](auto && b) -> json { return b.to_json(); }, *spec);

    if (use_colors)
    {
        j["use_colors"] = true;
    }

    return j;
}

Blueprint::Signal::Signal(json const & j)
{
    JsonDigester<Signal> d;

    d.require("name", [](json const & j, Signal & s) { s.name = get_signal_id(j); });

    d.require("type", [](json const & j, Signal & s) {
        std::map<std::string, Type> types;
        types["virtual"] = Type::virt;
        types["item"] = Type::item;
        types["fluid"] = Type::fluid;
        try
        {
            s.type = types.at(j);
        }
        catch (std::out_of_range & e)
        {
            throw std::runtime_error(FILE_LINE + ": " + j.dump());
        }
    });

    d.digest(j, *this);
}

json Blueprint::Signal::to_json() const
{
    json j;
    j["name"] = get_signal_name(name);
    if (type == Type::virt)
    {
        j["type"] = "virtual";
    }
    else if (type == Type::item)
    {
        j["type"] = "item";
    }
    else
    {
        assert(type == Type::fluid);
        j["type"] = "fluid";
    }
    return j;
}

Blueprint::Entity::Position::Position(json const & j)
{
    JsonDigester<Position> d;

    d.require("x", [](json const & j, Position & p) { p.x = j; });
    d.require("y", [](json const & j, Position & p) { p.y = j; });

    d.digest(j, *this);
}

json Blueprint::Entity::Position::to_json() const
{
    json j;
    j["x"] = x;
    j["y"] = y;
    return j;
}
