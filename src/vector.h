#ifndef __VECTOR_H
#define __VECTOR_H

#include <stddef.h>

/* elem_copy stores one element into a vector slot. It may deep-copy pointed
 * resources owned by the source element and must return 0 on success.
 * elem_destroy releases resources owned by one stored element. */
typedef void (*vector_elem_destroy_fn)(void *elem);
typedef int (*vector_elem_copy_fn)(void *dst, const void *src);

typedef struct {
    size_t capacity;
    size_t size;
    size_t elem_size;
    vector_elem_destroy_fn elem_destroy;
    vector_elem_copy_fn elem_copy;
    unsigned char *data;
} vector;

#define VECTOR_INIT_CAPACITY 8

#define vector_empty(v) ((v)->size == 0)
#define vector_size(v) ((v)->size)
#define vector_capacity(v) ((v)->capacity)

/* Example:
 *   vector_push_value(v, int, 42);
 *   *vector_get_as(v, 0, int) == 42;
 */
#define vector_front_as(v, T) ((T *)vector_front((v)))
#define vector_front_const_as(v, T) ((const T *)vector_front_const((v)))
#define vector_get_as(v, i, T) ((T *)vector_at((v), (i)))
#define vector_get_const_as(v, i, T) ((const T *)vector_at_const((v), (i)))
#define vector_back_as(v, T) ((T *)vector_back((v)))
#define vector_back_const_as(v, T) ((const T *)vector_back_const((v)))
#define vector_push_value(v, T, value) vector_push((v), &(T){ value })
#define vector_insert_value(v, i, T, value) vector_insert((v), (i), &(T){ value })
#define vector_set_value(v, i, T, value) vector_set((v), (i), &(T){ value })

/* Stack usage:
 *   vector v;
 *   vector_init(&v, sizeof(int), NULL, NULL);
 *   vector_destroy(&v);
 */
int vector_init(vector *v, size_t elem_size,
                vector_elem_destroy_fn elem_destroy,
                vector_elem_copy_fn elem_copy);
void vector_destroy(vector *v);

/* Heap usage:
 *   vector *v = vector_new(sizeof(int), NULL, NULL);
 *   vector_free(v);
 */
vector *vector_new(size_t elem_size,
                   vector_elem_destroy_fn elem_destroy,
                   vector_elem_copy_fn elem_copy);
void vector_free(vector *v);

int vector_copy(vector *dst, const vector *src);
vector *vector_clone(const vector *src);

/* Low-level api: deal with the address of element. */
int vector_reserve(vector *v, size_t capacity);
int vector_push(vector *v, const void *elem);
int vector_pop(vector *v);
int vector_insert(vector *v, size_t index, const void *elem);
int vector_set(vector *v, size_t index, const void *elem);
void vector_erase(vector *v, size_t index);
void vector_clear(vector *v);

/* Accessors return element addresses; use vector_get_as(...) for typed access. */
void *vector_front(vector *v);
const void *vector_front_const(const vector *v);
void *vector_at(vector *v, size_t index);
const void *vector_at_const(const vector *v, size_t index);
void *vector_back(vector *v);
const void *vector_back_const(const vector *v);

void vector_sort(vector *v, int (*cmp)(const void *, const void *));
void *vector_bsearch(const vector *v, const void *key,
                     int (*cmp)(const void *, const void *));

#endif
