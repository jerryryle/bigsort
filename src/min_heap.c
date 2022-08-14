#include "min_heap.h"
#include <assert.h>
#include <stdlib.h>

// Given an element index 'i', calculate the index of the parent element.
#define PARENT_ELEMENT(i)   (((i)-1) / 2)

// Given an element index 'i', calculate the index of the left child element.
#define LEFT_CHILD_ELEMENT(i)     ((2*(i)) + 1)

// Given an element index 'i', calculate the index of the right child element.
#define RIGHT_CHILD_ELEMENT(i)    ((2*(i)) + 2)

struct min_heap {
    struct min_heap_element *data;
    size_t element_count;
    size_t element_capacity;

    int (*compare)(const void *, const void *);
};

static void heapify(struct min_heap *heap);

static void swap_elements(struct min_heap *heap, size_t first_element, size_t second_element);

struct min_heap *min_heap_new(void *data, size_t data_size)
{
    assert(data);

    if (data_size < sizeof(struct min_heap_element)) {
        return NULL;
    }

    struct min_heap *heap = (struct min_heap *) malloc(sizeof(struct min_heap));
    if (!heap) {
        return NULL;
    }
    heap->data = (struct min_heap_element *) data;
    heap->element_count = 0;
    heap->element_capacity = data_size / sizeof(struct min_heap_element);

    return heap;
}

size_t min_heap_capacity(struct min_heap const *heap)
{
    assert(heap);
    return heap->element_capacity;
}

size_t min_heap_count(struct min_heap const *heap)
{
    assert(heap);
    return heap->element_count;
}

bool min_heap_is_full(struct min_heap const *heap)
{
    assert(heap);
    return heap->element_count >= heap->element_capacity;
}

bool min_heap_add(struct min_heap *heap, uint32_t key, FILE *value)
{
    assert(heap);
    if (heap->element_count >= heap->element_capacity) {
        return false;
    }

    // Get the index for a new element at the end of the array
    size_t current_element = heap->element_count;
    // Increase the element count to account for the new element
    heap->element_count++;

    // Put the new element data at the end of the array.
    heap->data[current_element].key = key;
    heap->data[current_element].value = value;

    // As long as the current element is smaller than its parent
    // Bubble it upwards until we hit the top of the tree
    while ((current_element != 0) &&
           (heap->data[current_element].key < heap->data[PARENT_ELEMENT(current_element)].key)) {
        // If the current element is smaller than its parent, swap it and keep moving upwards.
        size_t parent_element = PARENT_ELEMENT(current_element);
        swap_elements(heap, current_element, parent_element);
        current_element = parent_element;
    }
    return true;
}

bool min_heap_pop(struct min_heap *heap, uint32_t *key, FILE **value)
{
    assert(heap);
    assert(key);
    assert(value);

    if (heap->element_count <= 0) {
        return false;
    }

    // Retrieve element's value
    *key = heap->data[0].key;
    *value = heap->data[0].value;
    // Move the last element to the top and decrement the number of elements.
    heap->data[0] = heap->data[heap->element_count - 1];
    heap->element_count--;
    // re-heapify
    heapify(heap);

    return true;
}

void min_heap_clear(struct min_heap *heap)
{
    // Set the number of elements to zero. This effectively clears the heap.
    heap->element_count = 0;
}

void min_heap_delete(struct min_heap *heap)
{
    free(heap);
}

static void heapify(struct min_heap *heap)
{
    // If there's zero elements or only one element in the heap, there's nothing to do.
    if (heap->element_count <= 1) {
        return;
    }

    // Start with the top of the heap
    size_t current_element = 0;

    // Keep moving downwards until the current element is the minimum element.
    for (;;) {
        // Begin by assuming the current is the min element.
        size_t min_element = current_element;

        // Calculate the array indices of the left and right elements
        size_t const left_element = LEFT_CHILD_ELEMENT(current_element);
        size_t const right_element = RIGHT_CHILD_ELEMENT(current_element);

        // If there's a left child element, and if it's smaller than the current element, capture the left child element
        // as the minimum.
        if ((left_element < heap->element_count) && (heap->data[left_element].key < heap->data[current_element].key)) {
            min_element = left_element;
        }
        // If there's a right child element, and if it's smaller than both the current element and the left child
        // element, capture the right child element as the minimum.
        if ((right_element < heap->element_count) && (heap->data[right_element].key < heap->data[min_element].key)) {
            min_element = right_element;
        }

        if (min_element == current_element) {
            // If the current element is the smallest element, we're done.
            break;
        }

        // If the current element is not the smallest element, swap it with the smallest and proceed downwards.
        swap_elements(heap, min_element, current_element);

        // The min element has been swapped with the current element. Update current_element such that we perform
        // the next iteration starting at the position of the old min_element.
        current_element = min_element;
    }
}

static void swap_elements(struct min_heap *heap, size_t first_element, size_t second_element)
{
    struct min_heap_element const temp_element = heap->data[first_element];
    heap->data[first_element] = heap->data[second_element];
    heap->data[second_element] = temp_element;
}
