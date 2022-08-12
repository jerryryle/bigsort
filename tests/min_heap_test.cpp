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

TEST_F(MinHeapTest, CannotAddMoreElementsThanHeapCapacity) {

    EXPECT_FALSE(min_heap_is_full(heap));

    size_t capacity = min_heap_capacity(heap);
    for (size_t i = 0; i < capacity; i++) {
        EXPECT_TRUE(min_heap_add(heap, (uint32_t)i, nullptr));
    }
    EXPECT_TRUE(min_heap_is_full(heap));
    EXPECT_FALSE(min_heap_add(heap, (uint32_t)capacity, nullptr));
}

TEST_F(MinHeapTest, CannotPopFromEmptyHeap) {
    uint32_t key = 0;
    FILE *value = nullptr;
    EXPECT_FALSE(min_heap_pop(heap, &key, &value));
}

TEST_F(MinHeapTest, CanPopAddedElement) {
    uint32_t key = 0;
    FILE *value = nullptr;
    EXPECT_TRUE(min_heap_add(heap, 42, (FILE *)0x12345678));
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 42);
    EXPECT_EQ(value, (FILE *)0x12345678);
}

TEST_F(MinHeapTest, CannotPopMoreElementsThanAdded) {
    uint32_t key = 0;
    FILE *value = nullptr;
    EXPECT_TRUE(min_heap_add(heap, 42, (FILE *)0x12345678));
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 42);
    EXPECT_EQ(value, (FILE *)0x12345678);
    EXPECT_FALSE(min_heap_pop(heap, &key, &value));
}

TEST_F(MinHeapTest, SmallestElementInsertedLastMovesToTopOfHeap) {
    uint32_t key = 0;
    FILE *value = nullptr;

    // The heap has 0 elements
    EXPECT_EQ(min_heap_count(heap), 0);

    // 42 is added to the end of the heap and it becomes the top of heap since it's the only element.
    EXPECT_TRUE(min_heap_add(heap, 42, (FILE *)0x00000001));
    // 0 is added to the end of the heap and it becomes the top of heap since it's smaller than 42.
    EXPECT_TRUE(min_heap_add(heap, 0, (FILE *)0x00000002));

    // The heap now has 2 elements
    EXPECT_EQ(min_heap_count(heap), 2);

    // 0 is popped from the heap and 42 becomes the new top of heap.
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 0);
    EXPECT_EQ(value, (FILE *)0x00000002);

    // 42 is popped from the heap and the heap is now empty
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 42);
    EXPECT_EQ(value, (FILE *)0x00000001);

    // The heap is now empty
    EXPECT_EQ(min_heap_count(heap), 0);
}

TEST_F(MinHeapTest, HeapIsMaintainedAsElementsAreAddedAndRemoved) {
    uint32_t key = 0;
    FILE *value = nullptr;

    // The heap has 0 elements
    EXPECT_EQ(min_heap_count(heap), 0);

    // 43 is added to the heap and it becomes the top of heap since it's the only element.
    /* Heap:
             43
    */
    EXPECT_TRUE(min_heap_add(heap, 43, (FILE *)0x00000001));
    // 42 is added to the end of the heap and, because it is smaller than the 43 at the top, it is swapped with 43.
    /* Heap:
             43               42
            /        ->      /
          42               43
    */
    EXPECT_TRUE(min_heap_add(heap, 42, (FILE *)0x00000002));
    // 41 is added to the end of the heap and, because it is smaller than the 42 above it, it is swapped with 42.
    /* Heap:
             42             41
            /  \     ->    /  \
          43    41       43    42
    */
    EXPECT_TRUE(min_heap_add(heap, 41, (FILE *)0x00000003));
    // 0 is added to the end of the heap and, it moves upwards to become the top of the heap
    /* Heap:
             42             41             0
            /  \     ->    /  \     ->    /  \
          43    41        0   42        41    42
         /              /              /
        0             43             43
    */
    EXPECT_TRUE(min_heap_add(heap, 0, (FILE *)0x00000004));

    // The heap now has 4 elements
    EXPECT_EQ(min_heap_count(heap), 4);

    // 0 is popped and 43 is temporarily moves to the top and then pushed downwards.
    /* Heap:
             0              43             41
            /  \     ->    /  \     ->    /  \
          41    42       41    42       43    42
         /
        43
    */
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 0);
    EXPECT_EQ(value, (FILE *)0x00000004);

    // 41 is popped and 42 moves to the top and stays there.
    /* Heap:
             41             42
            /  \     ->    /
          43    42       43
    */
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 41);
    EXPECT_EQ(value, (FILE *)0x00000003);

    // 42 is popped from the heap and 43 moves to the top.
    /* Heap:
             42             43
            /        ->
          43
    */
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 42);
    EXPECT_EQ(value, (FILE *)0x00000002);

    // 43 is popped from the heap and the heap is now empty
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 43);
    EXPECT_EQ(value, (FILE *)0x00000001);

    // The heap is now empty
    EXPECT_EQ(min_heap_count(heap), 0);
}

TEST_F(MinHeapTest, CanClearHeap) {
    size_t capacity = min_heap_capacity(heap);
    for (size_t i = 0; i < capacity; i++) {
        EXPECT_TRUE(min_heap_add(heap, (uint32_t)i, nullptr));
    }
    // Heap is full. Cannot add another element.
    EXPECT_FALSE(min_heap_add(heap, (uint32_t)capacity, nullptr));

    // Clear heap
    min_heap_clear(heap);

    // Can now add another element.
    EXPECT_TRUE(min_heap_add(heap, (uint32_t)capacity, nullptr));
}
