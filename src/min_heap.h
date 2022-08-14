#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct min_heap;

struct min_heap_element {
    uint32_t key;
    FILE *value;
};

struct min_heap *min_heap_new(void *data, size_t data_size);

size_t min_heap_capacity(struct min_heap const *heap);

size_t min_heap_count(struct min_heap const *heap);

bool min_heap_is_full(struct min_heap const *heap);

bool min_heap_add(struct min_heap *heap, uint32_t key, FILE *value);

bool min_heap_pop(struct min_heap *heap, uint32_t *key, FILE **value);

void min_heap_clear(struct min_heap *heap);

void min_heap_delete(struct min_heap *heap);

#endif // MIN_HEAP_H
