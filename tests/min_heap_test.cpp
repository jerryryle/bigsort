#include "gtest/gtest.h"
#include <array>

extern "C" {
#include "min_heap.h"
}

static size_t const MAX_TEST_ELEMENTS = 32;
static size_t const TEST_BUFFER_SIZE = MAX_TEST_ELEMENTS * sizeof(struct min_heap_element);

class MinHeapTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        heap = min_heap_new(buffer.data(), buffer.size());
        EXPECT_TRUE(heap != nullptr);
    }

    void TearDown() override
    {
        min_heap_delete(heap);
        heap = nullptr;
    }

    std::array<char, TEST_BUFFER_SIZE> buffer{0};
    struct min_heap *heap{nullptr};
};


TEST_F(MinHeapTest, NewHeapCapacityAndCountAreCorrect)
{
    EXPECT_EQ(min_heap_capacity(heap), MAX_TEST_ELEMENTS);
    EXPECT_EQ(min_heap_count(heap), 0);
}

TEST_F(MinHeapTest, CannotCreateHeapWithInsufficientDataSize)
{
    // Create a buffer that cannot fit even a single element
    size_t const SMALL_BUFFER_SIZE = sizeof(struct min_heap_element) - 1;
    std::array<char, SMALL_BUFFER_SIZE> buffer{0};

    struct min_heap *heap = min_heap_new(buffer.data(), buffer.size());
    EXPECT_TRUE(heap == nullptr);
}

TEST_F(MinHeapTest, CannotAddMoreElementsThanHeapCapacity)
{

    EXPECT_FALSE(min_heap_is_full(heap));

    size_t capacity = min_heap_capacity(heap);
    for (size_t i = 0; i < capacity; i++) {
        EXPECT_TRUE(min_heap_add(heap, (uint32_t) i, nullptr));
    }
    EXPECT_TRUE(min_heap_is_full(heap));
    EXPECT_FALSE(min_heap_add(heap, (uint32_t) capacity, nullptr));
}

TEST_F(MinHeapTest, CannotPopFromEmptyHeap)
{
    uint32_t key = 0;
    FILE *value = nullptr;
    EXPECT_FALSE(min_heap_pop(heap, &key, &value));
}

TEST_F(MinHeapTest, CanPopAddedElement)
{
    uint32_t key = 0;
    FILE *value = nullptr;
    EXPECT_TRUE(min_heap_add(heap, 42, (FILE *) 0x12345678));
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 42);
    EXPECT_EQ(value, (FILE *) 0x12345678);
}

TEST_F(MinHeapTest, CannotPopMoreElementsThanAdded)
{
    uint32_t key = 0;
    FILE *value = nullptr;
    EXPECT_TRUE(min_heap_add(heap, 42, (FILE *) 0x12345678));
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 42);
    EXPECT_EQ(value, (FILE *) 0x12345678);
    EXPECT_FALSE(min_heap_pop(heap, &key, &value));
}

TEST_F(MinHeapTest, SmallestElementInsertedLastMovesToTopOfHeap)
{
    uint32_t key = 0;
    FILE *value = nullptr;

    // The heap has 0 elements
    EXPECT_EQ(min_heap_count(heap), 0);

    // 42 is added to the end of the heap and it becomes the top of heap since it's the only element.
    EXPECT_TRUE(min_heap_add(heap, 42, (FILE *) 0x00000001));
    // 0 is added to the end of the heap and it becomes the top of heap since it's smaller than 42.
    EXPECT_TRUE(min_heap_add(heap, 0, (FILE *) 0x00000002));

    // The heap now has 2 elements
    EXPECT_EQ(min_heap_count(heap), 2);

    // 0 is popped from the heap and 42 becomes the new top of heap.
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 0);
    EXPECT_EQ(value, (FILE *) 0x00000002);

    // 42 is popped from the heap and the heap is now empty
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 42);
    EXPECT_EQ(value, (FILE *) 0x00000001);

    // The heap is now empty
    EXPECT_EQ(min_heap_count(heap), 0);
}

TEST_F(MinHeapTest, HeapIsMaintainedAsElementsAreAddedAndRemoved)
{
    uint32_t key = 0;
    FILE *value = nullptr;

    // The heap has 0 elements
    EXPECT_EQ(min_heap_count(heap), 0);

    // 100 is added to the heap. It becomes the top of heap since it's the only element.
    /* Heap:
             100
    */
    EXPECT_TRUE(min_heap_add(heap, 100, (FILE *) 0x00000001));

    // 50 is added to the end of the heap and, because it is smaller than the 100 at the top, it is swapped with 100.
    /* Heap:
            100                50
            /         ->      /
          50                100
    */
    EXPECT_TRUE(min_heap_add(heap, 50, (FILE *) 0x00000002));

    // 200 is added to the end of the heap and, because it is larger than the 50 above it, it stays at the end.
    /* Heap:
             50              50
            /   \     ->    /   \
          100   200       100   200
    */
    EXPECT_TRUE(min_heap_add(heap, 200, (FILE *) 0x00000003));

    // 0 is added to the end of the heap and it moves upwards to become the top of the heap.
    /* Heap:
             50              50               0
            /   \     ->    /   \     ->    /   \
          100   200        0    200       50    200
         /               /              /
        0              100            100
    */
    EXPECT_TRUE(min_heap_add(heap, 0, (FILE *) 0x00000004));

    // 150 is added to the end of the heap and, because it is larger than the 50 above it, it stays at the end.
    /* Heap:
              0               0
            /   \     ->    /   \
          50    200       50    200
         /   \           /   \
       100   150       100   150
    */
    EXPECT_TRUE(min_heap_add(heap, 150, (FILE *) 0x00000005));

    // 160 is added to the end of the heap and, because it is smaller than the 200 above it, is swapped with 200.
    /* Heap:
                0                    0
            /       \     ->     /       \
          50        200        50        160
         /   \      /         /   \      /
       100   150  160       100   150  200
    */
    EXPECT_TRUE(min_heap_add(heap, 160, (FILE *) 0x00000006));

    // The heap now has 6 elements
    EXPECT_EQ(min_heap_count(heap), 6);

    // 0 is popped and 200, which is the end of the heap, moves to the top and then downwards.
    /* Heap:
                0                   200                  50                   50
            /       \     ->     /       \     ->     /       \     ->     /       \
          50        160        50        160        200       160        100       160
         /   \      /         /   \                /   \                /   \
       100   150  200       100   150            100   150            200   150
    */
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 0);
    EXPECT_EQ(value, (FILE *) 0x00000004);

    // 50 is popped and 150, which is the end of the heap, moves to the top and then downwards.
    /* Heap:
               50                   150                  100
            /       \     ->     /       \     ->     /       \
          100       160        100       160        150       160
         /   \                /                    /
       200   150            200                  200
    */
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 50);
    EXPECT_EQ(value, (FILE *) 0x00000002);

    // 100 is popped and 200, which is the end of the heap, moves to the top and then downwards.
    /* Heap:
               100                  200                  150
            /       \     ->     /       \     ->     /       \
          150       160        150       160        200       160
         /
       200
    */
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 100);
    EXPECT_EQ(value, (FILE *) 0x00000001);

    // 150 is popped and 160, which is the end of the heap, moves to the top and stays there.
    /* Heap:
               150                  160
            /       \     ->     /
          200       160        200
    */
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 150);
    EXPECT_EQ(value, (FILE *) 0x00000005);

    // 160 is popped and 200, which is the end of the heap, moves to the top and stays there.
    /* Heap:
               160                  200
            /             ->
          200
    */
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 160);
    EXPECT_EQ(value, (FILE *) 0x00000006);

    // 200 is popped from the heap and the heap is now empty
    EXPECT_TRUE(min_heap_pop(heap, &key, &value));
    EXPECT_EQ(key, 200);
    EXPECT_EQ(value, (FILE *) 0x00000003);

    // The heap is now empty
    EXPECT_EQ(min_heap_count(heap), 0);
}

TEST_F(MinHeapTest, CanClearHeap)
{
    size_t capacity = min_heap_capacity(heap);
    for (size_t i = 0; i < capacity; i++) {
        EXPECT_TRUE(min_heap_add(heap, (uint32_t) i, nullptr));
    }
    // Heap is full. Cannot add another element.
    EXPECT_FALSE(min_heap_add(heap, (uint32_t) capacity, nullptr));

    // Clear heap
    min_heap_clear(heap);

    // Can now add another element.
    EXPECT_TRUE(min_heap_add(heap, (uint32_t) capacity, nullptr));
}
