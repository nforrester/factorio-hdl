#include "Blueprint.h"
#include "Base64.h"
#include "zlib_easy.h"
#include "JsonDigester.h"

#include <cassert>

#define FILE_LINE std::string(__FILE__ ":" + std::to_string(__LINE__))

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

Blueprint::Blueprint(std::string const & blueprint_string)
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

        d.require("version", [](json const & j, Blueprint & bp) { bp.version = j; });

        d.digest(j, bp);
    });

    d.digest(json::parse(blueprint_string_to_raw_json(blueprint_string)), *this);
}

Blueprint::Icon::Icon(json const & j)
{
    JsonDigester<Icon> d;

    d.require("index", [](json const & j, Icon & i) { i.index = j; });
    d.require("signal", [](json const & j, Icon & i){ i.signal = std::make_optional(j); });

    d.digest(j, *this);
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
            assert(e.ports.emplace(std::stoi(port_string), pj).second);
        }
    });

    d.optional("control_behavior", [](json const & j, Entity & e)
    {
        e.control_behavior = std::make_optional(j);
    });

    d.optional("direction", [](json const & j, Entity & e) { e.direction = j; });
    d.require("name", [](json const & j, Entity & e) { e.name = j; });

    d.optional("position", [](json const & j, Entity & e)
    {
        e.position = std::make_optional(j);
    });

    d.digest(j, *this);
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

Blueprint::Entity::Port::Wire::Wire(json const & j)
{
    JsonDigester<Wire> d;

    d.require("entity_id", [](json const & j, Wire & w){ w.entity_id = j; });
    d.optional("circuit_id", [](json const & j, Wire & w){ w.port_num = j; });

    d.digest(j, *this);
}

Blueprint::Entity::ControlBehavior::ControlBehavior(json const & j)
{
    JsonDigester<ControlBehavior> d;

    d.optional("arithmetic_conditions", [](json const & j, ControlBehavior & c)
    {
        assert(!c.behavior.has_value());
        c.behavior = std::make_optional(ArithmeticConditions(j));
    });

    d.optional("decider_conditions", [](json const & j, ControlBehavior & c)
    {
        assert(!c.behavior.has_value());
        c.behavior = std::make_optional(DeciderConditions(j));
    });

    d.optional("filters", [](json const & j, ControlBehavior & c)
    {
        assert(!c.behavior.has_value());
        c.behavior = std::make_optional(Filters(j));
    });

    d.digest(j, *this);

    assert(behavior.has_value());
}

Blueprint::Entity::ControlBehavior::ArithmeticConditions::ArithmeticConditions(json const & j)
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
        // TODO make common with serialization
        std::map<std::string, ArithmeticCombinator::Op> ops;
        ops["+"] = ArithmeticCombinator::Op::ADD;
        ops["-"] = ArithmeticCombinator::Op::SUB;
        ops["*"] = ArithmeticCombinator::Op::MUL;
        ops["/"] = ArithmeticCombinator::Op::DIV;
        ops["%"] = ArithmeticCombinator::Op::MOD;
        ops["^"] = ArithmeticCombinator::Op::POW;
        ops["<<"] = ArithmeticCombinator::Op::LSH;
        ops[">>"] = ArithmeticCombinator::Op::RSH;
        ops["AND"] = ArithmeticCombinator::Op::AND;
        ops["OR"] = ArithmeticCombinator::Op::OR;
        ops["XOR"] = ArithmeticCombinator::Op::XOR;
        try
        {
            a.op = ops.at(j);
        }
        catch (std::out_of_range & e)
        {
            throw std::runtime_error(FILE_LINE + ": " + j.dump());
        }
    });

    d.digest(j, *this);

    assert(rhs_signal.has_value() || rhs_const.has_value());
}

Blueprint::Entity::ControlBehavior::DeciderConditions::DeciderConditions(json const & j)
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
        // TODO make common with serialization
        std::map<std::string, DeciderCombinator::Op> ops;
        ops["<"] = DeciderCombinator::Op::LT;
        ops[">"] = DeciderCombinator::Op::GT;
        ops["<="] = DeciderCombinator::Op::LE;
        ops[">="] = DeciderCombinator::Op::GE;
        ops["=="] = DeciderCombinator::Op::EQ;
        ops["!="] = DeciderCombinator::Op::NE;
        try
        {
            a.op = ops.at(j);
        }
        catch (std::out_of_range & e)
        {
            throw std::runtime_error(FILE_LINE + ": " + j.dump());
        }
    });

    d.digest(j, *this);

    assert(rhs_signal.has_value() || rhs_const.has_value());
}

Blueprint::Entity::ControlBehavior::Filters::Filters(json const & j)
{
    assert(j.is_array());

    for (json const & jj : j)
    {
        filters.emplace_back(jj);
    }
}

Blueprint::Entity::ControlBehavior::Filters::Filter::Filter(json const & j)
{
    JsonDigester<Filter> d;

    d.require("count", [](json const & j, Filter & f){ f.count = j; });
    d.require("index", [](json const & j, Filter & f){ f.index = j; });
    d.require("signal", [](json const & j, Filter & f){ f.signal = std::make_optional(j); });

    d.digest(j, *this);
}

Blueprint::Signal::Signal(json const & j)
{
    JsonDigester<Signal> d;

    d.require("name", [](json const & j, Signal & s) { s.name = j; });

    d.require("type", [](json const & j, Signal & s) {
        std::map<std::string, Type> types;
        types["virtual"] = Type::virt;
        types["item"] = Type::item;
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

Blueprint::Entity::Position::Position(json const & j)
{
    JsonDigester<Position> d;

    d.require("x", [](json const & j, Position & p) { p.x = j; });
    d.require("y", [](json const & j, Position & p) { p.y = j; });

    d.digest(j, *this);
}
