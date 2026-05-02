#ifndef __HEAP_TYPED_H
#define __HEAP_TYPED_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define HEAP_INIT_CAPACITY 8

#define heap_def(T, name)   \
    typedef struct {        \
        size_t capacity;    \
        size_t size;        \
        int (*compare)(const T a, const T b); \
        T *data;            \
    } heap_##name

#define heap_type_bytes(v) sizeof(*((v)->data))
#define heap_empty(v) ((v)->size == 0)
#define heap_size(v) ((v)->size)
#define heap_capacity(v) ((v)->capacity)

#define heap_back(v) ((v)->data[(v)->size-1])
#define heap_top(v) ((v)->data[0])

#define heap_new(v, cmp) ((v) = calloc(1, sizeof(*(v))), (v)->compare = (cmp), (v))

#define heap_free(v)                    \
    do {                                \
        if ((v) == NULL) return;        \
        if ((v)->data) free((v)->data); \
        free(v);                        \
    } while (0)

#define heap_expand(v)                                                \
    do {                                                              \
        if ((v)->capacity > (v)->size) break;                         \
        size_t _cap = (v)->capacity == 0 ? HEAP_INIT_CAPACITY : (v)->capacity * 2; \
        void *_p = realloc((v)->data, _cap * heap_type_bytes(v));     \
        if (_p == NULL) exit(1);                                      \
        (v)->capacity = _cap;                                         \
        (v)->data = _p;                                               \
    } while (0)

#define heap_push(v, e)                                     \
    do {                                                    \
        heap_expand(v);                                     \
        size_t _heap_i = (v)->size++;                       \
        while (_heap_i > 0) {                               \
            size_t _heap_parent = (_heap_i - 1) / 2;        \
            if ((v)->compare((v)->data[_heap_parent], e) >= 0) break; \
            (v)->data[_heap_i] = (v)->data[_heap_parent];   \
            _heap_i = _heap_parent;                         \
        }                                                   \
        (v)->data[_heap_i] = e;                             \
    } while (0)

#define heap_pop(v)                                         \
    do {                                                    \
        assert(!heap_empty(v));                             \
        (v)->data[0] = heap_back(v);                        \
        (v)->size--;                                        \
        size_t _heap_i = 0;                                 \
        while (1) {                                         \
            size_t _heap_left = _heap_i * 2 + 1;            \
            size_t _heap_right = _heap_left + 1;            \
            if (_heap_left >= heap_size(v)) break;          \
            size_t _heap_j = _heap_left;                    \
            if (_heap_right < heap_size(v) &&               \
                (v)->compare((v)->data[_heap_left], (v)->data[_heap_right]) < 0) { \
                _heap_j = _heap_right;                      \
            }                                               \
            if ((v)->compare((v)->data[_heap_i], (v)->data[_heap_j]) >= 0) break; \
            (v)->data[(v)->size] = (v)->data[_heap_i];      \
            (v)->data[_heap_i] = (v)->data[_heap_j];        \
            (v)->data[_heap_j] = (v)->data[(v)->size];      \
            _heap_i = _heap_j;                              \
        }                                                   \
    } while (0)

//      (type, name)
heap_def(int, int);
heap_def(unsigned int, uint);
heap_def(long long, ll);
heap_def(unsigned long long, ull);
heap_def(double, double);
heap_def(char*, str);
heap_def(void*, ptr);

#endif
