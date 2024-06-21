#ifndef __VECTOR_H
#define __VECTOR_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_INIT_CAPACITY 16

#define vector_def(T, name)   \
    typedef struct {          \
        size_t capacity;      \
        size_t size;          \
        T *data;              \
    } vector_##name

#define vector_new(name) calloc(1, sizeof(vector_##name))

#define vector_free(v)     \
    do {                   \
        free((v)->data);   \
        free(v);           \
    } while (0)

#define vector_type_bytes(v) sizeof(*((v)->data))

#define vector_empty(v) ((v)->size == 0)

#define vector_size(v) ((v)->size)

#define vector_back(v) ((v)->data[(v)->size-1])

#define vector_expand(v)                                              \
    do {                                                              \
        if ((v)->capacity > (v)->size) break;                         \
        size_t _cap = (v)->capacity == 0 ? VECTOR_INIT_CAPACITY : (v)->capacity * 2; \
        void *_p = realloc((v)->data, _cap * vector_type_bytes(v));   \
        if (_p == NULL) exit(1);                                      \
        (v)->capacity = _cap;                                         \
        (v)->data = _p;                                               \
    } while (0)

#define vector_push_back(v, e)              \
    do {                                    \
        vector_expand(v);                   \
        (v)->data[(v)->size++] = e;         \
    } while (0)

#define vector_pop_back(v)        \
    do {                          \
        assert(!vector_empty(v)); \
        (v)->size--;              \
    } while (0)

#define vector_clear(v)  \
    do {                 \
        (v)->size = 0;   \
    } while (0)

#define vector_insert(v, i, e)                               \
    do {                                                     \
        assert((i) < (v)->size);                             \
        vector_expand(v);                                    \
        memmove(&((v)->data[(i)+1]), &((v)->data[(i)]),      \
            (vector_size(v) - (i)) * vector_type_bytes(v));  \
        (v)->data[(i)] = e;                                  \
        (v)->size++;                                         \
    } while (0)

#define vector_erase(v, i)                                   \
    do {                                                     \
        assert((i) < (v)->size);                             \
        const size_t _n = (v)->size - (i) - 1;               \
        if (_n > 0) {                                        \
            memmove(&((v)->data[(i)]), &((v)->data[(i)+1]),  \
                _n * vector_type_bytes(v));                  \
        }                                                    \
        (v)->size--;                                         \
    } while (0)

#define vector_sort(v, cmp)   \
    (qsort((v)->data, (v)->size, vector_type_bytes(v), cmp))

//        (type, name)
vector_def(int, int);
vector_def(unsigned int, uint);
vector_def(long long, long);
vector_def(unsigned long long, ulong);
vector_def(double, double);
vector_def(const char *, str);
vector_def(void *, ptr);

#endif
