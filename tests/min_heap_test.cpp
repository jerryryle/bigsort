#include "gtest/gtest.h"
#include <array>

extern "C" {
#include "min_heap.h"
}

static size_t const MAX_TEST_ELEMENTS = 32;
static size_t const TEST_BUFFER_SIZE = MAX_TEST_ELEMENTS * sizeof(struct min_heap_element);

class MinHeapTest : public ::testing::Test {
protected:
    void SetUp() override {
        heap = min_heap_new(buffer.data(), buffer.size());
        EXPECT_TRUE(heap != nullptr);
    }

    void TearDown() override {
        min_heap_delete(heap);
        heap = nullptr;
    }

    std::array<char, TEST_BUFFER_SIZE> buffer {0};
    struct min_heap *heap {nullptr};
};


TEST_F(MinHeapTest, NewHeapCapacityAndCountAreCorrect) {
    EXPECT_EQ(min_heap_capacity(heap), MAX_TEST_ELEMENTS);
    EXPECT_EQ(min_heap_count(heap), 0);
}

TEST_F(MinHeapTest, CannotCreateHeapWithInsufficientDataSize) {
    // Create a buffer that cannot fit even a single element
    size_t const SMALL_BUFFER_SIZE = sizeof(struct min_heap_element) - 1;
    std::array<char, SMALL_BUFFER_SIZE> buffer {0};

    struct min_heap *heap = min_heap_new(buffer.data(), buffer.size());
    EXPECT_TRUE(heap == nullptr);
}
