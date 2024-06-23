#ifndef __HEAP_H
#define __HEAP_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define HEAP_INIT_CAPACITY 16

#define heap_def(T, name)   \
    typedef struct {        \
        size_t capacity;    \
        size_t size;        \
        /* second param not use a pointer to support heap_push macro */ \
        int (*compare)(const T *a, const T b); \
        T *data;            \
    } heap_##name

#define heap_new(v, cmp) calloc(1, sizeof(*v)); v->compare = cmp

#define heap_free(v)       \
    do {                   \
        free((v)->data);   \
        free(v);           \
    } while (0)

#define heap_type_bytes(v) sizeof(*((v)->data))

#define heap_empty(v) ((v)->size == 0)

#define heap_size(v) ((v)->size)

#define heap_back(v) ((v)->data[(v)->size-1])

#define heap_top(v) ((v)->data[0])

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
        assert((v)->compare);                               \
        heap_expand(v);                                     \
        size_t i = (v)->size++;                             \
        while (i > 0) {                                     \
            size_t parent = (i - 1) / 2;                    \
            if ((v)->compare((v)->data + parent, e) >= 0) break; \
            (v)->data[i] = (v)->data[parent];               \
            i = parent;                                     \
        }                                                   \
        (v)->data[i] = e;                                   \
    } while (0)

#define heap_pop(v)                                         \
    do {                                                    \
        assert((v)->compare);                               \
        assert(!heap_empty(v));                             \
        (v)->data[0] = heap_back(v);                        \
        (v)->size--;                                        \
        size_t i = 0;                                       \
        while (1) {                                         \
            size_t left = i*2 + 1;                          \
            size_t right = left + 1;                        \
            if (left >= heap_size(v)) break;                \
            size_t j = left;                                \
            if (right >= heap_size(v)) {                    \
                if ((v)->compare((v)->data + left, (v)->data[right]) < 0) { \
                    j = right;                              \
                }                                           \
            }                                               \
            if ((v)->compare((v)->data + i, (v)->data[j]) >= 0) break; \
            (v)->data[(v)->size] = (v)->data[i];            \
            (v)->data[i] = (v)->data[j];                    \
            (v)->data[j] = (v)->data[(v)->size];            \
            i = j;                                          \
        }                                                   \
    } while (0)

//        (type, name)
heap_def(int, int);
heap_def(unsigned int, uint);
heap_def(long long, long);
heap_def(unsigned long long, ulong);
heap_def(double, double);
heap_def(char *, str);
heap_def(void *, ptr);

#endif
