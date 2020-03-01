#include "gtest/gtest.h"

#include "Bits.h"

TEST(BitsTest, Basic)
{
    Bits bits(10);
    EXPECT_FALSE(bits.any());
    bits.set(3);
    EXPECT_TRUE(bits.any());
    EXPECT_FALSE(bits.get(0));
    EXPECT_FALSE(bits.get(1));
    EXPECT_FALSE(bits.get(2));
    EXPECT_TRUE(bits.get(3));
    EXPECT_FALSE(bits.get(4));
    EXPECT_FALSE(bits.get(5));
    EXPECT_FALSE(bits.get(6));
    EXPECT_FALSE(bits.get(7));
    EXPECT_FALSE(bits.get(8));
    EXPECT_FALSE(bits.get(9));
    bits.clear();
    EXPECT_FALSE(bits.any());

    bits.set(0);
    bits.set(5);
    bits.set(9);

    Bits bits2(10);
    bits2.set(2);
    bits2.set(5);

    bits &= bits2;

    EXPECT_FALSE(bits.get(0));
    EXPECT_FALSE(bits.get(1));
    EXPECT_FALSE(bits.get(2));
    EXPECT_FALSE(bits.get(3));
    EXPECT_FALSE(bits.get(4));
    EXPECT_TRUE(bits.get(5));
    EXPECT_FALSE(bits.get(6));
    EXPECT_FALSE(bits.get(7));
    EXPECT_FALSE(bits.get(8));
    EXPECT_FALSE(bits.get(9));

    bits = bits2;

    EXPECT_FALSE(bits.get(0));
    EXPECT_FALSE(bits.get(1));
    EXPECT_TRUE(bits.get(2));
    EXPECT_FALSE(bits.get(3));
    EXPECT_FALSE(bits.get(4));
    EXPECT_TRUE(bits.get(5));
    EXPECT_FALSE(bits.get(6));
    EXPECT_FALSE(bits.get(7));
    EXPECT_FALSE(bits.get(8));
    EXPECT_FALSE(bits.get(9));
}

TEST(BitsTest, Which)
{
    for (size_t size = 1; size < 5000; ++size)
    {
        Bits bits(size);
        size_t n_sets = rand() % size;
        for (size_t i = 0; i < n_sets; ++i)
        {
            bits.set(rand() % size);
        }
        for (int i = size - 1; i >= 0; --i)
        {
        }
        std::vector<size_t> which;
        bits.which(which);
        size_t which_idx = 0;
        for (size_t i = 0; i < size; ++i)
        {
            if (bits.get(i))
            {
                ASSERT_TRUE(which_idx < which.size());
                ASSERT_EQ(i, which[which_idx]);
                ++which_idx;
            }
            else
            {
                if (which_idx < which.size())
                {
                    ASSERT_NE(i, which[which_idx]);
                }
            }
        }
        ASSERT_EQ(which_idx, which.size());
    }
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
