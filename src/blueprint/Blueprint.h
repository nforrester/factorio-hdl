#pragma once

#include "json.hpp"
#include "src/entities/ArithmeticCombinator.h"
#include "src/entities/DeciderCombinator.h"

#include <string>
#include <optional>
#include <variant>

using json = nlohmann::json;

std::string blueprint_string_to_raw_json(std::string const & blueprint_string);
std::string raw_json_to_blueprint_string(std::string const & raw_json);

struct Blueprint
{
    Blueprint(std::string const & blueprint_string);

    struct Signal
    {
        Signal(json const & j);

        SignalId name;

        enum class Type
        {
            virt,
            item,
        } type;
    };

    struct Icon
    {
        Icon(json const & j);

        int index;
        std::optional<Signal> signal; // TODO TRY TO UNOPTION
    };

    std::vector<Icon> icons;

    struct Entity
    {
        Entity(json const & j);

        int id;

        struct Port
        {
            Port(json const & j);

            struct Wire
            {
                Wire(json const & j);

                int entity_id;
                int port_num = 1;
            };

            std::vector<Wire> green;
            std::vector<Wire> red;
        };

        std::map<int, Port> ports;

        struct DeciderConditions
        {
            DeciderConditions(json const & j);

            std::optional<Signal> lhs; // TODO TRY TO UNOPTION
            std::optional<Signal> rhs_signal; // TODO TRY TO UNOPTION
            std::optional<SignalValue> rhs_const; // TODO TRY TO UNOPTION
            std::optional<Signal> out; // TODO TRY TO UNOPTION
            DeciderCombinator::Op op;
            bool copy_count_from_input;
        };

        struct ArithmeticConditions
        {
            ArithmeticConditions(json const & j);

            std::optional<Signal> lhs; // TODO TRY TO UNOPTION
            std::optional<Signal> rhs_signal; // TODO TRY TO UNOPTION
            std::optional<SignalValue> rhs_const; // TODO TRY TO UNOPTION
            std::optional<Signal> out; // TODO TRY TO UNOPTION
            ArithmeticCombinator::Op op;
        };

        struct Filters
        {
            Filters(json const & j);

            struct Filter
            {
                Filter(json const & j);

                SignalValue count;
                int index;
                std::optional<Signal> signal; // TODO TRY TO UNOPTION
            };

            std::vector<Filter> filters;
        };

        std::optional<std::variant<DeciderConditions, ArithmeticConditions, Filters>> control_behavior; // TODO TRY TO UNOPTION

        int direction = 0;

        SignalId name;

        struct Position
        {
            Position(json const & j);

            double x;
            double y;
        };

        std::optional<Position> position; // TODO TRY TO UNOPTION
    };
    std::map<int, Entity> entities;

    std::string item;

    int64_t version;
};
