#include "gtest/gtest.h"

#include "src/Factorio.h"
#include "ConstantCombinator.h"
#include "Hysteresis.h"

TEST(CircuitTest, HysteresisGreenDeadBandHalf3)
{
    int constexpr stability_time = 4;

    Factorio fac;

    auto & c = fac.new_entity<ConstantCombinator>();
    c.constants.add(Signal::iron_plate, 0);

    auto & h = fac.new_entity<Hysteresis>(fac,
                                          Signal::iron_plate,
                                          Signal::copper_plate,
                                          10,
                                          3,
                                          Wire::green);

    /*
     *  05 06 07 08 09 10 11 12 13 14 15
     *           **************
     *             DEAD  BAND
     */

    fac.connect(Wire::green, c.port, h.in_port());
    fac.lock(Wire::green, h.out_port());

    fac.build();

    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(0, fac.read(h.out_port(), Wire::green).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, 6);
    ASSERT_EQ(6, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(0, fac.read(h.out_port(), Wire::green).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, 1);
    ASSERT_EQ(7, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(0, fac.read(h.out_port(), Wire::green).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, 5);
    ASSERT_EQ(12, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(0, fac.read(h.out_port(), Wire::green).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, 1);
    ASSERT_EQ(13, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(1, fac.read(h.out_port(), Wire::green).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, -1);
    ASSERT_EQ(12, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(1, fac.read(h.out_port(), Wire::green).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, -4);
    ASSERT_EQ(8, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(1, fac.read(h.out_port(), Wire::green).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, -1);
    ASSERT_EQ(7, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(0, fac.read(h.out_port(), Wire::green).get(Signal::copper_plate));
}

TEST(CircuitTest, HysteresisRedDeadBandHalf4)
{
    int constexpr stability_time = 4;

    Factorio fac;

    auto & c = fac.new_entity<ConstantCombinator>();
    c.constants.add(Signal::iron_plate, 0);

    auto & h = fac.new_entity<Hysteresis>(fac,
                                          Signal::iron_plate,
                                          Signal::copper_plate,
                                          10,
                                          4,
                                          Wire::red);

    /*
     *  05 06 07 08 09 10 11 12 13 14 15
     *        ********************
     *             DEAD  BAND
     */

    fac.connect(Wire::red, c.port, h.in_port());
    fac.lock(Wire::red, h.out_port());

    fac.build();

    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(0, fac.read(h.out_port(), Wire::red).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, 6);
    ASSERT_EQ(6, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(0, fac.read(h.out_port(), Wire::red).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, 1);
    ASSERT_EQ(7, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(0, fac.read(h.out_port(), Wire::red).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, 6);
    ASSERT_EQ(13, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(0, fac.read(h.out_port(), Wire::red).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, 1);
    ASSERT_EQ(14, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(1, fac.read(h.out_port(), Wire::red).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, -1);
    ASSERT_EQ(13, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(1, fac.read(h.out_port(), Wire::red).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, -6);
    ASSERT_EQ(7, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(1, fac.read(h.out_port(), Wire::red).get(Signal::copper_plate));

    c.constants.add(Signal::iron_plate, -1);
    ASSERT_EQ(6, c.constants.get(Signal::iron_plate));
    EXPECT_TRUE(fac.run_until_stable(stability_time));
    EXPECT_EQ(0, fac.read(h.out_port(), Wire::red).get(Signal::copper_plate));
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
