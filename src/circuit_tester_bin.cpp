#include "Factorio.h"
#include "util.h"
#include "fdl/FdlEntity.h"
#include "fdl/FdlMacro.h"
#include "fdl/util.h"
#include "entities/ConstantCombinator.h"

#include <iostream>
#include <sstream>

int main(int argc, char ** argv)
{
    assert(argc == 2);

    Factorio fac;

    S::PtrV test_description = S::consume(read_file(argv[1]), argv[1], 1);
    for (auto const & form : test_description)
    {
        Fdl::check_valid_top_level_form(*form);
    }
    Fdl::expand_all_macros(test_description);

    struct TestPort
    {
        WireColor color;
        bool input;
        SignalValue value;
        ConstantCombinator * entity = nullptr;
    };
    std::unordered_map<std::string, TestPort> test_ports;

    std::vector<std::pair<TestPort *, SignalId>> test_input_format;
    std::vector<std::pair<TestPort *, SignalId>> test_output_format;
    std::vector<std::vector<SignalValue>> test_inputs;
    std::vector<std::vector<SignalValue>> test_outputs;

    std::string filename;
    int latency = -1;
    bool parts_created = false;

    for (S::Ptr const & s : test_description)
    {
        S::PtrV const & test_form = s->as_list()->l;
        std::string command = test_form.front()->as_symbol()->s;
        if (command == "load")
        {
            if (test_form.size() != 2 ||
                !test_form.at(1)->as_string())
            {
                throw S::ParseError(s->file, s->line, "Expected (load <filename>)");
            }
            filename = test_form.at(1)->as_string()->s;
            continue;
        }

        if (command == "green" || command == "red")
        {
            if (test_form.size() != 3 ||
                !test_form.at(1)->as_symbol() ||
                !(test_form.at(1)->as_symbol()->s == "input" ||
                  test_form.at(1)->as_symbol()->s == "output") ||
                !test_form.at(2)->as_symbol())
            {
                throw S::ParseError(
                    s->file,
                    s->line,
                    "Expected (" + command + " <input|output> <symbol>)");
            }

            if (parts_created)
            {
                throw S::ParseError(
                    s->file,
                    s->line,
                    "Test ports must be declared before the part under test.");
            }

            TestPort test_port;
            test_port.color = command == "green" ? Wire::green : Wire::red;
            test_port.input = "input" == test_form.at(1)->as_symbol()->s;

            std::string name = test_form.at(2)->as_symbol()->s;

            if (test_ports.count(name))
            {
                throw S::ParseError(
                    s->file,
                    s->line,
                    "Duplicate test port: " + name);
            }
            test_ports[name] = test_port;

            continue;
        }

        if (command == "test-type")
        {
            if (test_form.size() != 3 ||
                !test_form.at(1)->as_symbol() ||
                test_form.at(1)->as_symbol()->s != "fixed-latency" ||
                !test_form.at(2)->as_int())
            {
                throw S::ParseError(
                    s->file,
                    s->line,
                    "Only fixed-latency tests are currently supported. "
                    "Expected (test-type fixed-latency <int>)");
            }

            latency = test_form.at(2)->as_int()->v;
        }

        if (command == "part-under-test")
        {
            if (test_form.size() != 2 ||
                !test_form.at(1)->as_list() ||
                !test_form.at(1)->as_list()->l.size() == 0 ||
                !test_form.at(1)->as_list()->l.front()->as_symbol())
            {
                throw S::ParseError(
                    s->file,
                    s->line,
                    "Expected (part-under-test <instantiation>)");
            }

            if (parts_created)
            {
                throw S::ParseError(
                    s->file,
                    s->line,
                    "There can be only one part under test.");
            }
            parts_created = true;

            std::unordered_map<std::string, std::set<WireColor>> wire_colors;
            for (auto const & np : test_ports)
            {
                wire_colors[np.first].insert(np.second.color);
                test_ports.at(np.first).entity = &fac.new_entity<ConstantCombinator>();
            }

            std::unordered_set<std::string> wire_names;
            for (auto const & np : test_ports)
            {
                wire_names.insert(np.first);
            }

            Fdl::Entity & part_under_test = fac.new_entity<Fdl::Entity>(
                test_form.at(1)->as_list()->l.front()->as_symbol()->s,
                Fdl::gather_new_part_args(*test_form.at(1),
                                          std::unordered_map<std::string, SignalId>(),
                                          std::unordered_map<std::string, SignalValue>(),
                                          wire_names),
                wire_colors,
                filename);

            for (auto const & np : part_under_test.ports())
            {
                Port & inner_port = *np.second;
                for (auto const & test_port_name :
                     part_under_test.wires_desired_by_port(inner_port))
                {
                    TestPort const & test_port = test_ports.at(test_port_name);
                    Port & outer_port = *test_port.entity->ports().at("out");
                    fac.connect(test_port.color, outer_port, inner_port);
                }
            }

            continue;
        }

        if (command == "test")
        {
            S::PtrV const & test_sequence = s->as_list()->l;
            if (test_sequence.size() < 3 ||
                !test_sequence.at(1)->as_list() ||
                !std::all_of(test_sequence.at(1)->as_list()->l.begin(),
                             test_sequence.at(1)->as_list()->l.end(),
                             [](S::Ptr const & x) -> bool {
                                 return x->as_list() &&
                                        x->as_list()->l.size() == 2 &&
                                        x->as_list()->l.at(0)->as_symbol() &&
                                        x->as_list()->l.at(1)->as_symbol();
                             }) ||
                !std::all_of(
                    test_sequence.begin() + 2,
                    test_sequence.end(),
                    [](S::Ptr const & x) -> bool {
                        return x->as_list() &&
                               x->as_list()->l.size() == 2 &&
                               std::all_of(
                                x->as_list()->l.begin(),
                                x->as_list()->l.end(),
                                [](S::Ptr const & y) -> bool {
                                    return y->as_list() &&
                                           std::all_of(
                                            y->as_list()->l.begin(),
                                            y->as_list()->l.end(),
                                            [](S::Ptr const & x) -> bool {
                                                return x->as_int();
                                            });
                                });
                    }))
            {
                throw S::ParseError(
                    s->file,
                    s->line,
                    "Malformed test expression."); // TODO make this less vague
            }

            assert(false); // TODO assemble test_input_format, test_output_format, test_inputs, and test_outputs.
        }
    }

    assert(parts_created);
    assert(test_input_format.size() > 0 ||
           test_output_format.size() > 0);
    assert(latency >= 0);

    fac.build();

    assert(false); // TODO run the test

    return 0;
}
