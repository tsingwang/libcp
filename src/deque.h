#ifndef __DEQUE_H
#define __DEQUE_H

#include <stddef.h>

typedef void (*deque_elem_destroy_fn)(void *elem);
typedef int (*deque_elem_copy_fn)(void *dst, const void *src);

typedef struct {
    size_t capacity;
    size_t size;
    size_t head;
    size_t elem_size;
    deque_elem_destroy_fn elem_destroy;
    deque_elem_copy_fn elem_copy;
    unsigned char *data;
} deque;

#define DEQUE_INIT_CAPACITY 8

#define deque_empty(q) ((q)->size == 0)
#define deque_size(q) ((q)->size)
#define deque_capacity(q) ((q)->capacity)

/* Example:
 *   deque_push_back_value(q, int, 1);
 *   deque_push_front_value(q, int, 0);
 *   *deque_front_as(q, int) == 0;
 *   *deque_back_as(q, int) == 1;
 */
#define deque_get_as(q, i, T) ((T *)deque_at((q), (i)))
#define deque_get_const_as(q, i, T) ((const T *)deque_at_const((q), (i)))
#define deque_front_as(q, T) ((T *)deque_front((q)))
#define deque_front_const_as(q, T) ((const T *)deque_front_const((q)))
#define deque_back_as(q, T) ((T *)deque_back((q)))
#define deque_back_const_as(q, T) ((const T *)deque_back_const((q)))
#define deque_push_back_value(q, T, value) deque_push_back((q), &(T){ value })
#define deque_push_front_value(q, T, value) deque_push_front((q), &(T){ value })
#define deque_set_value(q, i, T, value) deque_set((q), (i), &(T){ value })

/* Stack usage:
 *   deque q;
 *   deque_init(&q, sizeof(int), NULL, NULL);
 *   deque_destroy(&q);
 */
int deque_init(deque *q, size_t elem_size,
               deque_elem_destroy_fn elem_destroy,
               deque_elem_copy_fn elem_copy);
void deque_destroy(deque *q);

/* Heap usage:
 *   deque *q = deque_new(sizeof(int), NULL, NULL);
 *   deque_free(q);
 */
deque *deque_new(size_t elem_size,
                 deque_elem_destroy_fn elem_destroy,
                 deque_elem_copy_fn elem_copy);
void deque_free(deque *q);

int deque_copy(deque *dst, const deque *src);
deque *deque_clone(const deque *src);

/* Low-level api: deal with the address of element. */
int deque_reserve(deque *q, size_t capacity);
int deque_push_back(deque *q, const void *elem);
int deque_push_front(deque *q, const void *elem);
int deque_pop_back(deque *q);
int deque_pop_front(deque *q);
int deque_set(deque *q, size_t index, const void *elem);
void deque_clear(deque *q);

/* Accessors return element addresses; use deque_front_as/back_as/get_as for typed access. */
void *deque_at(deque *q, size_t index);
const void *deque_at_const(const deque *q, size_t index);
void *deque_front(deque *q);
const void *deque_front_const(const deque *q);
void *deque_back(deque *q);
const void *deque_back_const(const deque *q);

#endif
