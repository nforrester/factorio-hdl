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
    std::string to_blueprint_string() const;

    Blueprint(std::string const & blueprint_string);
    json to_json() const;

    struct Signal
    {
        Signal(json const & j);
        json to_json() const;

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
        json to_json() const;

        int index;
        std::optional<Signal> signal;
    };

    std::vector<Icon> icons;

    struct Entity
    {
        Entity() {}
        Entity(json const & j);
        json to_json() const;

        int id;

        struct Port
        {
            Port(json const & j);
            json to_json() const;

            struct Wire
            {
                Wire(json const & j);
                json to_json() const;

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
            json to_json() const;

            std::optional<Signal> lhs;
            std::optional<Signal> rhs_signal;
            std::optional<SignalValue> rhs_const;
            std::optional<Signal> out;
            DeciderCombinator::Op op;
            bool copy_count_from_input;
        };

        struct ArithmeticConditions
        {
            ArithmeticConditions(json const & j);
            json to_json() const;

            std::optional<Signal> lhs;
            std::optional<Signal> rhs_signal;
            std::optional<SignalValue> rhs_const;
            std::optional<Signal> out;
            ArithmeticCombinator::Op op;
        };

        struct Filters
        {
            Filters(json const & j);
            json to_json() const;

            struct Filter
            {
                Filter(json const & j);
                json to_json() const;

                SignalValue count;
                int index;
                std::optional<Signal> signal;
            };

            std::vector<Filter> filters;
        };

        std::optional<std::variant<DeciderConditions, ArithmeticConditions, Filters>> control_behavior;

        int direction = 0;

        SignalId name;

        struct Position
        {
            Position(double x_, double y_): x(x_), y(y_) {}
            Position(json const & j);
            json to_json() const;

            bool operator==(Position const & other) const
            {
                return x == other.x && y == other.y;
            }

            double x;
            double y;
        };

        std::optional<Position> position;
    };
    std::map<int, Entity> entities;

    std::string item;

    int64_t version;
};
