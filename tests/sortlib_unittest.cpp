#include "gtest/gtest.h"

class BigSortTest : public ::testing::Test {
protected:

    BigSortTest() {
        // You can do set-up work for each test here.
    }

    ~BigSortTest() override {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    // Class members declared here can be used by all tests in the test suite
};

TEST_F(BigSortTest, DoesXyz) {
    EXPECT_EQ(1, 0);
}
