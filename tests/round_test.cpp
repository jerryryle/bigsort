#include "gtest/gtest.h"

extern "C" {
#include "round.h"
}

TEST(RoundTest, RoundsUp)
{
    EXPECT_EQ(round_up_to_multiple_of_4(1), 4);
    EXPECT_EQ(round_up_to_multiple_of_4(2), 4);
    EXPECT_EQ(round_up_to_multiple_of_4(3), 4);
    EXPECT_EQ(round_up_to_multiple_of_4(5), 8);
}

TEST(RoundTest, ExistingMultipleOfFourDoesNotRoundUp)
{
    EXPECT_EQ(round_up_to_multiple_of_4(4), 4);
}

TEST(RoundTest, ZeroStaysZero)
{
    EXPECT_EQ(round_up_to_multiple_of_4(0), 0);
}
