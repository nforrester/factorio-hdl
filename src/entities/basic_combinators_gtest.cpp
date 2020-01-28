#include "gtest/gtest.h"

#include "src/Factorio.h"
#include "ConstantCombinator.h"
#include "DeciderCombinator.h"
#include "ArithmeticCombinator.h"

TEST(CircuitTest, ConstantCombinator)
{
    Factorio fac;

    auto & c = fac.new_entity<ConstantCombinator>();

    c.constants.add(Signal::iron_plate, 10);
    c.constants.add(Signal::copper_plate, 3);
    c.constants.add(Signal::underground_belt, -9);

    fac.lock(Wire::green, c.port("out"));

    fac.build();

    EXPECT_TRUE(fac.run_until_stable(2));

    EXPECT_EQ(10, fac.read(c.port("out")).get(Signal::iron_plate));
    EXPECT_EQ(3, fac.read(c.port("out")).get(Signal::copper_plate));
    EXPECT_EQ(-9, fac.read(c.port("out")).get(Signal::underground_belt));

    c.constants.add(Signal::copper_plate, -3);

    EXPECT_TRUE(fac.run_until_stable(2));

    EXPECT_EQ(10, fac.read(c.port("out")).get(Signal::iron_plate));
    EXPECT_EQ(0, fac.read(c.port("out")).get(Signal::copper_plate));
    EXPECT_EQ(-9, fac.read(c.port("out")).get(Signal::underground_belt));
}

TEST(CircuitTest, DeciderCombinator)
{
    std::array<DeciderCombinator::Op, 6> ops = {
        DeciderCombinator::Op::GT,
        DeciderCombinator::Op::LT,
        DeciderCombinator::Op::GE,
        DeciderCombinator::Op::LE,
        DeciderCombinator::Op::EQ,
        DeciderCombinator::Op::NE,
    };

    using bool6 = std::array<bool, 6>;

    std::vector<std::tuple<SignalValue, SignalValue, bool6>> test_cases;
    test_cases.emplace_back( 0,  5, bool6({0, 1, 0, 1, 0, 1}));
    test_cases.emplace_back( 5,  0, bool6({1, 0, 1, 0, 0, 1}));
    test_cases.emplace_back(-5,  5, bool6({0, 1, 0, 1, 0, 1}));
    test_cases.emplace_back( 5, -5, bool6({1, 0, 1, 0, 0, 1}));
    test_cases.emplace_back( 5,  5, bool6({0, 0, 1, 1, 1, 0}));
    test_cases.emplace_back( 0,  0, bool6({0, 0, 1, 1, 1, 0}));

    for (auto const & test_case : test_cases)
    {
        SignalValue lhs, rhs;
        bool6 results;
        std::tie(lhs, rhs, results) = test_case;
        for (int i = 0; i < 6; ++i)
        {
            for (int write_input_value = 0; write_input_value < 2; ++write_input_value)
            {
                Factorio fac;

                auto & c = fac.new_entity<ConstantCombinator>();
                c.constants.add(Signal::iron_plate, lhs);
                c.constants.add(Signal::copper_plate, rhs);

                auto & d = fac.new_entity<DeciderCombinator>(
                    Signal::iron_plate,
                    ops[i],
                    Signal::copper_plate,
                    Signal::iron_plate,
                    write_input_value);

                fac.connect(Wire::green, c.port("out"), d.port("in"));
                fac.lock(Wire::green, d.port("out"));

                fac.build();

                EXPECT_TRUE(fac.run_until_stable(3));

                EXPECT_EQ(!results.at(i) ? 0 : write_input_value ? lhs : 1, fac.read(d.port("out")).get(Signal::iron_plate));
            }
        }
    }
}

TEST(CircuitTest, ArithmeticCombinator)
{
    std::array<ArithmeticCombinator::Op, 11> ops = {
        ArithmeticCombinator::Op::ADD,
        ArithmeticCombinator::Op::SUB,
        ArithmeticCombinator::Op::MUL,
        ArithmeticCombinator::Op::DIV,
        ArithmeticCombinator::Op::MOD,
        ArithmeticCombinator::Op::POW,
        ArithmeticCombinator::Op::LSH,
        ArithmeticCombinator::Op::RSH,
        ArithmeticCombinator::Op::AND,
        ArithmeticCombinator::Op::OR,
        ArithmeticCombinator::Op::XOR,
    };

    using int11 = std::array<SignalValue, 11>;

    std::vector<std::tuple<SignalValue, SignalValue, int11>> test_cases;
    test_cases.emplace_back(  0,   5, int11({ 5,  -5,   0,  0,  0,         0,         0,  0,  0,  5,   5}));
    test_cases.emplace_back(  5,   0, int11({ 5,   5,   0,  0,  0,         1,         5,  5,  0,  5,   5}));
    test_cases.emplace_back( -5,   5, int11({ 0, -10, -25, -1,  0,     -3125,      -160, -1,  1, -1,  -2}));
    test_cases.emplace_back(  5,  -5, int11({ 0,  10, -25, -1,  0,         0, 671088640,  0,  1, -1,  -2}));
    test_cases.emplace_back(  5,   5, int11({10,   0,  25,  1,  0,      3125,       160,  0,  5,  5,   0}));
    test_cases.emplace_back(  0,   0, int11({ 0,   0,   0,  0,  0,         1,         0,  0,  0,  0,   0}));
    test_cases.emplace_back( -5,  18, int11({13, -23, -90,  0, -5, 766306777,  -1310720, -1, 18, -5, -23}));

    for (auto const & test_case : test_cases)
    {
        SignalValue lhs, rhs;
        int11 results;
        std::tie(lhs, rhs, results) = test_case;
        for (int i = 0; i < 11; ++i)
        {
            Factorio fac;

            auto & c = fac.new_entity<ConstantCombinator>();
            c.constants.add(Signal::iron_plate, lhs);
            c.constants.add(Signal::copper_plate, rhs);

            auto & d = fac.new_entity<ArithmeticCombinator>(
                Signal::iron_plate,
                ops[i],
                Signal::copper_plate,
                Signal::iron_plate);

            fac.connect(Wire::green, c.port("out"), d.port("in"));
            fac.lock(Wire::green, d.port("out"));

            fac.build();

            EXPECT_TRUE(fac.run_until_stable(3));

            EXPECT_EQ(results.at(i), fac.read(d.port("out")).get(Signal::iron_plate))
                << lhs << " " << i << " " << rhs << " " << results.at(i);
        }
    }
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
