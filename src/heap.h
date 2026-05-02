#ifndef __HEAP_H
#define __HEAP_H

#include <stddef.h>

typedef void (*heap_elem_destroy_fn)(void *elem);
typedef int (*heap_elem_copy_fn)(void *dst, const void *src);
typedef int (*heap_elem_compare_fn)(const void *a, const void *b);

typedef struct {
    size_t capacity;
    size_t size;
    size_t elem_size;
    heap_elem_compare_fn elem_compare;
    heap_elem_destroy_fn elem_destroy;
    heap_elem_copy_fn elem_copy;
    unsigned char *data;
} heap;

#define HEAP_INIT_CAPACITY 8

#define heap_empty(h) ((h)->size == 0)
#define heap_size(h) ((h)->size)
#define heap_capacity(h) ((h)->capacity)

/* Example:
 *   heap_push_value(h, int, 3);
 *   heap_push_value(h, int, 5);
 *   *heap_top_as(h, int) == 5;
 */
#define heap_top_as(h, T) ((T *)heap_top((h)))
#define heap_top_const_as(h, T) ((const T *)heap_top_const((h)))
#define heap_push_value(h, T, value) heap_push((h), &(T){ value })

/* Stack usage:
 *   heap h;
 *   heap_init(&h, sizeof(int), int_cmp, NULL, NULL);
 *   heap_destroy(&h);
 */
int heap_init(heap *h, size_t elem_size,
              heap_elem_compare_fn elem_compare,
              heap_elem_destroy_fn elem_destroy,
              heap_elem_copy_fn elem_copy);
void heap_destroy(heap *h);

/* Heap usage:
 *   heap *h = heap_new(sizeof(int), int_cmp, NULL, NULL);
 *   heap_free(h);
 */
heap *heap_new(size_t elem_size,
               heap_elem_compare_fn elem_compare,
               heap_elem_destroy_fn elem_destroy,
               heap_elem_copy_fn elem_copy);
void heap_free(heap *h);

int heap_copy(heap *dst, const heap *src);
heap *heap_clone(const heap *src);

/* This is a max-heap: elem_compare(a, b) > 0 means a should stay above b. */
int heap_reserve(heap *h, size_t capacity);
int heap_push(heap *h, const void *elem);
int heap_pop(heap *h);
void heap_clear(heap *h);

/* Accessors return element addresses; use heap_top_as(...) for typed access. */
void *heap_top(heap *h);
const void *heap_top_const(const heap *h);

#endif
