#include "gtest/gtest.h"

#include "layout.h"

using namespace Blueprint;

/*
 * adjacent combinators are 1 block apart
 * combinators with one space in between are 2 block apart
 * 
 * for combinators of size 2, distance is measured from closest port,
 * not necessarily the port you're connecting to!
 * 
 * combinator -> combinator = 10 blocks apart
 * combinator -> steel pole = 10 blocks apart
 * steel pole -> steel pole =  9 blocks apart
 * 
 * Quarter circle combinator-combinator maximum range map:
 * 
 *   0123456789A
 * 
 * A XX
 * 9   XXXX
 * 8       X
 * 7        X
 * 6         X
 * 5          X
 * 4          X
 * 3          X
 * 2          X
 * 1           X
 * 0 X         X
 * 
 *   0123456789A
 */

TEST(ConnectionDistance, InRangeOfConstantCombinator)
{
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1, 10,  0, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1, 10,  1, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  9,  2, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  9,  3, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  9,  4, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  9,  5, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  8,  6, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  7,  7, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  6,  8, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  5,  9, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  4,  9, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  3,  9, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  2,  9, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  1, 10, 1, 1));
    EXPECT_TRUE(close_enough_for_direct_connection(0, 0, 1, 1,  0, 10, 1, 1));
}

TEST(ConnectionDistance, OutOfRangeOfConstantCombinator)
{
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1, 11,  0, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1, 11,  1, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1, 11,  2, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1, 10,  2, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1, 10,  3, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1, 10,  4, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1, 10,  5, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1, 10,  6, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  9,  6, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  9,  6, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  8,  7, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  8,  8, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  7,  8, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  6,  9, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  6, 10, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  5, 10, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  4, 10, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  3, 10, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  2, 10, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  2, 11, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  1, 11, 1, 1));
    EXPECT_FALSE(close_enough_for_direct_connection(0, 0, 1, 1,  0, 11, 1, 1));
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
