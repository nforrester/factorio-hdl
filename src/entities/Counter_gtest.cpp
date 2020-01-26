#include "gtest/gtest.h"

#include "src/Factorio.h"
#include "Counter.h"

TEST(CircuitTest, Counter1Green)
{
    Factorio fac;
    auto & c = fac.new_entity<Counter>(fac, Signal::iron_plate, 1, Wire::green);
    fac.lock(Wire::green, c.port());
    fac.build();
    EXPECT_FALSE(fac.run_until_stable(100));
    EXPECT_EQ(100, fac.read(c.port(), Wire::green).get(Signal::iron_plate));
}

TEST(CircuitTest, CounterMinus3Red)
{
    Factorio fac;
    auto & c = fac.new_entity<Counter>(fac, Signal::iron_plate, -3, Wire::red);
    fac.lock(Wire::red, c.port());
    fac.build();
    EXPECT_FALSE(fac.run_until_stable(100));
    EXPECT_EQ(-300, fac.read(c.port(), Wire::red).get(Signal::iron_plate));
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
