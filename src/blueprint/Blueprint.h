#pragma once

#include "src/util.h"
#include "json.hpp"
#include "util.h"
#include "src/entities/ArithmeticCombinator.h"
#include "src/entities/DeciderCombinator.h"

#include <string>
#include <optional>
#include <variant>

using json = nlohmann::json;

std::string blueprint_string_to_raw_json(std::string const & blueprint_string);
std::string raw_json_to_blueprint_string(std::string const & raw_json);

namespace Blueprint
{
    struct Signal
    {
        Signal(SignalId id):
            name(id),
            type(id < num_non_virtual_signals ?
                 (id < ::Signal::crude_oil ? Type::item : Type::fluid) :
                 Type::virt)
        {
        }

        Signal(json const & j);
        json to_json() const;

        SignalId name;

        enum class Type
        {
            virt,
            item,
            fluid,
        } type;
    };

    struct Icon
    {
        Icon(int index_, Signal const & signal_): index(index_), signal(signal_) {}
        Icon(json const & j);
        json to_json() const;

        int index;
        std::optional<Signal> signal;
    };

    struct Entity
    {
        Entity() {}
        Entity(json const & j);
        json to_json() const;

        int id;

        struct Port
        {
            Port() {}
            Port(json const & j);
            json to_json() const;

            struct Wire
            {
                Wire(int entity_id_, int port_num_): entity_id(entity_id_), port_num(port_num_) {}
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
            DeciderConditions() {}
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
            ArithmeticConditions() {}
            ArithmeticConditions(json const & j);
            json to_json() const;

            std::optional<Signal> lhs;
            std::optional<Signal> rhs_signal;
            std::optional<SignalValue> rhs_const;
            std::optional<Signal> out;
            ArithmeticCombinator::Op op;
        };

        struct CircuitCondition
        {
            CircuitCondition() {}
            CircuitCondition(json const & j);
            json to_json() const;

            std::optional<Signal> lhs;
            std::optional<Signal> rhs_signal;
            std::optional<SignalValue> rhs_const;
            DeciderCombinator::Op op;
        };

        struct Filters
        {
            Filters() {}
            Filters(json const & j);
            json to_json() const;

            struct Filter
            {
                Filter(SignalValue count_, int index_, Signal signal_):
                    count(count_), index(index_), signal(signal_) {}
                Filter(json const & j);
                json to_json() const;

                SignalValue count;
                int index;
                std::optional<Signal> signal;
            };

            std::vector<Filter> filters;
        };

        struct ControlBehavior
        {
            ControlBehavior() {}
            ControlBehavior(json const & j);
            json to_json() const;

            std::optional<std::variant<
                DeciderConditions,
                ArithmeticConditions,
                CircuitCondition,
                Filters>> spec;
            bool use_colors = false;
        };

        std::optional<ControlBehavior> control_behavior;

        int direction = 1;

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

    struct Blueprint
    {
        std::string to_blueprint_string() const;

        Blueprint();
        Blueprint(std::string const & blueprint_string);
        json to_json() const;

        std::vector<Icon> icons;

        std::map<int, Entity> entities;

        std::string item;
        std::string label;

        int64_t version;
    };
}
