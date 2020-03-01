#include "gtest/gtest.h"

#include "hilbert_curve.h"

TEST(HilbertCurve, SideLen)
{
    EXPECT_EQ(1, compute_final_side_length(0));
    EXPECT_EQ(1, compute_final_side_length(1));
    EXPECT_EQ(2, compute_final_side_length(2));
    EXPECT_EQ(2, compute_final_side_length(3));
    EXPECT_EQ(2, compute_final_side_length(4));
    EXPECT_EQ(4, compute_final_side_length(5));
    EXPECT_EQ(4, compute_final_side_length(16));
    EXPECT_EQ(8, compute_final_side_length(17));
}

TEST(HilbertCurve, Pos)
{
    EXPECT_EQ(std::make_pair(0, 0), compute_position_along_hilbert_curve(1, 0));

    EXPECT_EQ(std::make_pair(0, 0), compute_position_along_hilbert_curve(2, 0));
    EXPECT_EQ(std::make_pair(0, 1), compute_position_along_hilbert_curve(2, 1));
    EXPECT_EQ(std::make_pair(1, 1), compute_position_along_hilbert_curve(2, 2));
    EXPECT_EQ(std::make_pair(1, 0), compute_position_along_hilbert_curve(2, 3));

    EXPECT_EQ(std::make_pair(0, 0), compute_position_along_hilbert_curve(4,  0));
    EXPECT_EQ(std::make_pair(1, 0), compute_position_along_hilbert_curve(4,  1));
    EXPECT_EQ(std::make_pair(1, 1), compute_position_along_hilbert_curve(4,  2));
    EXPECT_EQ(std::make_pair(0, 1), compute_position_along_hilbert_curve(4,  3));
    EXPECT_EQ(std::make_pair(0, 2), compute_position_along_hilbert_curve(4,  4));
    EXPECT_EQ(std::make_pair(0, 3), compute_position_along_hilbert_curve(4,  5));
    EXPECT_EQ(std::make_pair(1, 3), compute_position_along_hilbert_curve(4,  6));
    EXPECT_EQ(std::make_pair(1, 2), compute_position_along_hilbert_curve(4,  7));
    EXPECT_EQ(std::make_pair(2, 2), compute_position_along_hilbert_curve(4,  8));
    EXPECT_EQ(std::make_pair(2, 3), compute_position_along_hilbert_curve(4,  9));
    EXPECT_EQ(std::make_pair(3, 3), compute_position_along_hilbert_curve(4, 10));
    EXPECT_EQ(std::make_pair(3, 2), compute_position_along_hilbert_curve(4, 11));
    EXPECT_EQ(std::make_pair(3, 1), compute_position_along_hilbert_curve(4, 12));
    EXPECT_EQ(std::make_pair(2, 1), compute_position_along_hilbert_curve(4, 13));
    EXPECT_EQ(std::make_pair(2, 0), compute_position_along_hilbert_curve(4, 14));
    EXPECT_EQ(std::make_pair(3, 0), compute_position_along_hilbert_curve(4, 15));
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
