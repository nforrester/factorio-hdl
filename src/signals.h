#pragma once

#include <cstdint>
#include <limits>

// TODO make rollover defined behavior
using SignalValue = int32_t;

using SignalId = uint_fast16_t;
SignalId constexpr num_non_virtual_signals = 210;
SignalId constexpr num_virtual_signals = 48;
SignalId constexpr num_signals = num_non_virtual_signals + num_virtual_signals;
namespace Signal
{
    SignalId constexpr wooden_chest                   = 0;
    SignalId constexpr iron_chest                     = 1;
    SignalId constexpr steel_chest                    = 2;
    SignalId constexpr storage_tank                   = 3;
    SignalId constexpr transport_belt                 = 4;
    SignalId constexpr fast_transport_belt            = 5;
    SignalId constexpr express_transport_belt         = 6;
    SignalId constexpr underground_belt               = 7;
    SignalId constexpr fast_underground_belt          = 8;
    SignalId constexpr express_underground_belt       = 9;
    SignalId constexpr splitter                       = 10;
    SignalId constexpr fast_splitter                  = 11;
    SignalId constexpr express_splitter               = 12;
    SignalId constexpr burner_inserter                = 13;
    SignalId constexpr inserter                       = 14;
    SignalId constexpr long_handed_inserter           = 15;
    SignalId constexpr fast_inserter                  = 16;
    SignalId constexpr filter_inserter                = 17;
    SignalId constexpr stack_inserter                 = 18;
    SignalId constexpr stack_filter_inserter          = 19;
    SignalId constexpr small_electric_pole            = 20;
    SignalId constexpr medium_electric_pole           = 21;
    SignalId constexpr big_electric_pole              = 22;
    SignalId constexpr substation                     = 23;
    SignalId constexpr pipe                           = 24;
    SignalId constexpr pipe_to_ground                 = 25;
    SignalId constexpr pump                           = 26;
    SignalId constexpr rail                           = 27;
    SignalId constexpr train_stop                     = 28;
    SignalId constexpr rail_signal                    = 29;
    SignalId constexpr rail_chain_signal              = 30;
    SignalId constexpr locomotive                     = 31;
    SignalId constexpr cargo_wagon                    = 32;
    SignalId constexpr fluid_wagon                    = 33;
    SignalId constexpr artillery_wagon                = 34;
    SignalId constexpr car                            = 35;
    SignalId constexpr tank                           = 36;
    SignalId constexpr logistic_robot                 = 37;
    SignalId constexpr construction_robot             = 38;
    SignalId constexpr active_provider_chest          = 39;
    SignalId constexpr passive_provider_chest         = 40;
    SignalId constexpr storage_chest                  = 41;
    SignalId constexpr buffer_chest                   = 42;
    SignalId constexpr requester_chest                = 43;
    SignalId constexpr roboport                       = 44;
    SignalId constexpr lamp                           = 45;
    SignalId constexpr red_wire                       = 46;
    SignalId constexpr green_wire                     = 47;
    SignalId constexpr arithmetic_combinator          = 48;
    SignalId constexpr decider_combinator             = 49;
    SignalId constexpr constant_combinator            = 50;
    SignalId constexpr power_switch                   = 51;
    SignalId constexpr programmable_speaker           = 52;
    SignalId constexpr stone_brick                    = 53;
    SignalId constexpr concrete                       = 54;
    SignalId constexpr hazard_concrete                = 55;
    SignalId constexpr refined_concrete               = 56;
    SignalId constexpr refined_hazard_concrete        = 57;
    SignalId constexpr landfill                       = 58;
    SignalId constexpr cliff_explosives               = 59;
    SignalId constexpr repair_pack                    = 60;
    SignalId constexpr blueprint                      = 61;
    SignalId constexpr deconstruction_planner         = 62;
    SignalId constexpr upgrade_planner                = 63;
    SignalId constexpr blueprint_book                 = 64;
    SignalId constexpr boiler                         = 65;
    SignalId constexpr steam_engine                   = 66;
    SignalId constexpr steam_turbine                  = 67;
    SignalId constexpr solar_panel                    = 68;
    SignalId constexpr accumulator                    = 69;
    SignalId constexpr nuclear_reactor                = 70;
    SignalId constexpr heat_exchanger                 = 71;
    SignalId constexpr heat_pipe                      = 72;
    SignalId constexpr burner_mining_drill            = 73;
    SignalId constexpr electric_mining_drill          = 74;
    SignalId constexpr offshore_pump                  = 75;
    SignalId constexpr pumpjack                       = 76;
    SignalId constexpr stone_furnace                  = 77;
    SignalId constexpr steel_furnace                  = 78;
    SignalId constexpr electric_furnace               = 79;
    SignalId constexpr assembling_machine_1           = 80;
    SignalId constexpr assembling_machine_2           = 81;
    SignalId constexpr assembling_machine_3           = 82;
    SignalId constexpr oil_refinery                   = 83;
    SignalId constexpr chemical_plant                 = 84;
    SignalId constexpr centrifuge                     = 85;
    SignalId constexpr lab                            = 86;
    SignalId constexpr beacon                         = 87;
    SignalId constexpr speed_module                   = 88;
    SignalId constexpr speed_module_2                 = 89;
    SignalId constexpr speed_module_3                 = 90;
    SignalId constexpr efficiency_module              = 91;
    SignalId constexpr efficiency_module_2            = 92;
    SignalId constexpr efficiency_module_3            = 93;
    SignalId constexpr productivity_module            = 94;
    SignalId constexpr productivity_module_2          = 95;
    SignalId constexpr productivity_module_3          = 96;
    SignalId constexpr wood                           = 97;
    SignalId constexpr coal                           = 98;
    SignalId constexpr stone                          = 99;
    SignalId constexpr iron_ore                       = 100;
    SignalId constexpr copper_ore                     = 101;
    SignalId constexpr uranium_ore                    = 102;
    SignalId constexpr raw_fish                       = 103;
    SignalId constexpr iron_plate                     = 104;
    SignalId constexpr copper_plate                   = 105;
    SignalId constexpr solid_fuel                     = 106;
    SignalId constexpr steel_plate                    = 107;
    SignalId constexpr plastic_bar                    = 108;
    SignalId constexpr sulfur                         = 109;
    SignalId constexpr battery                        = 110;
    SignalId constexpr explosives                     = 111;
    SignalId constexpr crude_oil_barrel               = 112;
    SignalId constexpr heavy_oil_barrel               = 113;
    SignalId constexpr light_oil_barrel               = 114;
    SignalId constexpr lubricant_barrel               = 115;
    SignalId constexpr petroleum_gas_barrel           = 116;
    SignalId constexpr sulfuric_acid_barrel           = 117;
    SignalId constexpr water_barrel                   = 118;
    SignalId constexpr copper_cable                   = 119;
    SignalId constexpr iron_stick                     = 120;
    SignalId constexpr iron_gear_wheel                = 121;
    SignalId constexpr empty_barrel                   = 122;
    SignalId constexpr electronic_circuit             = 123;
    SignalId constexpr advanced_circuit               = 124;
    SignalId constexpr processing_unit                = 125;
    SignalId constexpr engine_unit                    = 126;
    SignalId constexpr electric_engine_unit           = 127;
    SignalId constexpr flying_robot_frame             = 128;
    SignalId constexpr satellite                      = 129;
    SignalId constexpr rocket_part                    = 130;
    SignalId constexpr rocket_control_unit            = 131;
    SignalId constexpr low_density_structure          = 132;
    SignalId constexpr rocket_fuel                    = 133;
    SignalId constexpr nuclear_fuel                   = 134;
    SignalId constexpr uranium_235                    = 135;
    SignalId constexpr uranium_238                    = 136;
    SignalId constexpr uranium_fuel_cell              = 137;
    SignalId constexpr used_up_uranium_fuel_cell      = 138;
    SignalId constexpr automation_science_pack        = 139;
    SignalId constexpr logistic_science_pack          = 140;
    SignalId constexpr military_science_pack          = 141;
    SignalId constexpr chemical_science_pack          = 142;
    SignalId constexpr production_science_pack        = 143;
    SignalId constexpr utility_science_pack           = 144;
    SignalId constexpr space_science_pack             = 145;
    SignalId constexpr pistol                         = 146;
    SignalId constexpr submachine_gun                 = 147;
    SignalId constexpr shotgun                        = 148;
    SignalId constexpr combat_shotgun                 = 149;
    SignalId constexpr rocket_launcher                = 150;
    SignalId constexpr flamethrower                   = 151;
    SignalId constexpr land_mine                      = 152;
    SignalId constexpr firearm_magazine               = 153;
    SignalId constexpr piercing_rounds_magazine       = 154;
    SignalId constexpr uranium_rounds_magazine        = 155;
    SignalId constexpr shotgun_shells                 = 156;
    SignalId constexpr piercing_shotgun_shells        = 157;
    SignalId constexpr cannon_shell                   = 158;
    SignalId constexpr explosive_cannon_shell         = 159;
    SignalId constexpr uranium_cannon_shell           = 160;
    SignalId constexpr explosive_uranium_cannon_shell = 161;
    SignalId constexpr artillery_shell                = 162;
    SignalId constexpr rocket                         = 163;
    SignalId constexpr explosive_rocket               = 164;
    SignalId constexpr atomic_bomb                    = 165;
    SignalId constexpr flamethrower_ammo              = 166;
    SignalId constexpr grenade                        = 167;
    SignalId constexpr cluster_grenade                = 168;
    SignalId constexpr poison_capsule                 = 169;
    SignalId constexpr slowdown_capsule               = 170;
    SignalId constexpr defender_capsule               = 171;
    SignalId constexpr distractor_capsule             = 172;
    SignalId constexpr destroyer_capsule              = 173;
    SignalId constexpr discharge_defense_remote       = 174;
    SignalId constexpr artillery_targeting_remote     = 175;
    SignalId constexpr light_armor                    = 176;
    SignalId constexpr heavy_armor                    = 177;
    SignalId constexpr modular_armor                  = 178;
    SignalId constexpr power_armor                    = 179;
    SignalId constexpr power_armor_mk2                = 180;
    SignalId constexpr portable_solar_panel           = 181;
    SignalId constexpr portable_fusion_reactor        = 182;
    SignalId constexpr energy_shield                  = 183;
    SignalId constexpr energy_shield_mk2              = 184;
    SignalId constexpr personal_battery               = 185;
    SignalId constexpr personal_battery_mk2           = 186;
    SignalId constexpr personal_laser_defense         = 187;
    SignalId constexpr discharge_defense              = 188;
    SignalId constexpr belt_immunity_equipment        = 189;
    SignalId constexpr exoskeleton                    = 190;
    SignalId constexpr personal_roboport              = 191;
    SignalId constexpr personal_roboport_mk2          = 192;
    SignalId constexpr nightvision                    = 193;
    SignalId constexpr wall                           = 194;
    SignalId constexpr gate                           = 195;
    SignalId constexpr gun_turret                     = 196;
    SignalId constexpr laser_turret                   = 197;
    SignalId constexpr flamethrower_turret            = 198;
    SignalId constexpr artillery_turret               = 199;
    SignalId constexpr radar                          = 200;
    SignalId constexpr rocket_silo                    = 201;
    SignalId constexpr crude_oil                      = 202;
    SignalId constexpr heavy_oil                      = 203;
    SignalId constexpr light_oil                      = 204;
    SignalId constexpr lubricant                      = 205;
    SignalId constexpr petroleum_gas                  = 206;
    SignalId constexpr sulfuric_acid                  = 207;
    SignalId constexpr water                          = 208;
    SignalId constexpr steam                          = 209;
    SignalId constexpr digit_0                        = 210;
    SignalId constexpr digit_1                        = 211;
    SignalId constexpr digit_2                        = 212;
    SignalId constexpr digit_3                        = 213;
    SignalId constexpr digit_4                        = 214;
    SignalId constexpr digit_5                        = 215;
    SignalId constexpr digit_6                        = 216;
    SignalId constexpr digit_7                        = 217;
    SignalId constexpr digit_8                        = 218;
    SignalId constexpr digit_9                        = 219;
    SignalId constexpr letter_a                       = 220;
    SignalId constexpr letter_b                       = 221;
    SignalId constexpr letter_c                       = 222;
    SignalId constexpr letter_d                       = 223;
    SignalId constexpr letter_e                       = 224;
    SignalId constexpr letter_f                       = 225;
    SignalId constexpr letter_g                       = 226;
    SignalId constexpr letter_h                       = 227;
    SignalId constexpr letter_i                       = 228;
    SignalId constexpr letter_j                       = 229;
    SignalId constexpr letter_k                       = 230;
    SignalId constexpr letter_l                       = 231;
    SignalId constexpr letter_m                       = 232;
    SignalId constexpr letter_n                       = 233;
    SignalId constexpr letter_o                       = 234;
    SignalId constexpr letter_p                       = 235;
    SignalId constexpr letter_q                       = 236;
    SignalId constexpr letter_r                       = 237;
    SignalId constexpr letter_s                       = 238;
    SignalId constexpr letter_t                       = 239;
    SignalId constexpr letter_u                       = 240;
    SignalId constexpr letter_v                       = 241;
    SignalId constexpr letter_w                       = 242;
    SignalId constexpr letter_x                       = 243;
    SignalId constexpr letter_y                       = 244;
    SignalId constexpr letter_z                       = 245;
    SignalId constexpr color_red                      = 246;
    SignalId constexpr color_green                    = 247;
    SignalId constexpr color_blue                     = 248;
    SignalId constexpr color_yellow                   = 249;
    SignalId constexpr color_magenta                  = 250;
    SignalId constexpr color_cyan                     = 251;
    SignalId constexpr color_white                    = 252;
    SignalId constexpr color_grey                     = 253;
    SignalId constexpr color_black                    = 254;
    SignalId constexpr check                          = 255;
    SignalId constexpr info                           = 256;
    SignalId constexpr dot                            = 257;
}
static_assert(num_signals == Signal::dot + 1);
static_assert(Signal::digit_0 == num_non_virtual_signals);
static_assert(Signal::digit_0 == num_signals - num_virtual_signals);

namespace LogicSignal
{
    SignalId constexpr max_id = std::numeric_limits<SignalId>::max();

    SignalId constexpr everything = max_id - 1;
    SignalId constexpr anything = everything - 1;
    SignalId constexpr each = anything - 1;
    SignalId constexpr constant = each - 1;

    SignalId constexpr min_id = constant;
}

static_assert(num_signals <= LogicSignal::min_id);
