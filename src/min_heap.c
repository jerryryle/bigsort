#include "min_heap.h"
#include <assert.h>
#include <stdlib.h>

struct min_heap {
    char *data;
    size_t element_count;
    size_t element_capacity;
    size_t element_size;
    int (*compare)(const void *, const void *);
};

struct min_heap *min_heap_new(void *data, size_t data_size)
{
    assert(data);

    if (data_size < sizeof(struct min_heap_element)) {
        return NULL;
    }

    struct min_heap *heap = (struct min_heap *)malloc(sizeof(struct min_heap));
    if (!heap) {
        return NULL;
    }
    heap->data = (char *)data;
    heap->element_count = 0;
    heap->element_capacity = data_size/sizeof(struct min_heap_element);

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

bool min_heap_add(struct min_heap *heap, uint32_t key, FILE *value)
{
    assert(heap);
    return false;
}

void min_heap_delete(struct min_heap *heap)
{
    free(heap);
}
