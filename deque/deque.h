#ifndef __DEQUE_H
#define __DEQUE_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define DEQUE_INIT_CAPACITY 8

#define deque_def(T, name)   \
    typedef struct {         \
        size_t capacity;     \
        size_t head;         \
        size_t tail;         \
        T *data;             \
    } deque_##name

#define deque_type_bytes(q) sizeof(*((q)->data))
#define deque_empty(q) ((q)->head == (q)->tail)
#define deque_size(q) (((q)->tail >= (q)->head) ? (q)->tail - (q)->head : \
        (q)->capacity - (q)->head + (q)->tail)
#define deque_capacity(q) ((q)->capacity)

#define deque_front(q) ((q)->data[(q)->head])
#define deque_back(q) ((q)->tail > 0 ? (q)->data[(q)->tail-1] : (q)->data[(q)->capacity-1])

#define deque_new(q) calloc(1, sizeof(*(q)))

#define deque_free(q)      \
    do {                   \
        free((q)->data);   \
        free(q);           \
    } while (0)

#define deque_expand(q)                                                        \
    do {                                                                       \
        if (deque_capacity(q) > deque_size(q)) break;                          \
        size_t _cap = (q)->capacity == 0 ? DEQUE_INIT_CAPACITY : (q)->capacity * 2; \
        void *_dst = malloc(_cap * deque_type_bytes(q));                       \
        if (_dst == NULL) exit(1);                                             \
        if ((q)->tail < (q)->head) {                                           \
            int _bytes = ((q)->capacity - (q)->head) * deque_type_bytes(q);    \
            memcpy(_dst, (q)->data + (q)->head, _bytes);                       \
            memcpy((char *)_dst + _bytes, (q)->data, (q)->tail * deque_type_bytes(q)); \
        } else if ((q)->tail < (q)->head) {                                    \
            memcpy(_dst, (q)->data + (q)->head,                                \
                ((q)->tail - (q)->head) * deque_type_bytes(q));                \
        } else { /* nothing to do when initialize firstly */ }                 \
        (q)->head = 0;                                                         \
        (q)->tail = (q)->capacity == 0 ? 0 : (q)->capacity - 1;                \
        (q)->capacity = _cap;                                                  \
        free((q)->data);                                                       \
        (q)->data = _dst;                                                      \
    } while (0)

#define deque_push_back(q, e)            \
    do {                                 \
        deque_expand(q);                 \
        (q)->data[(q)->tail++] = e;      \
        if ((q)->tail == (q)->capacity)  \
            (q)->tail = 0;               \
    } while (0)

#define deque_pop_back(q)                     \
    do {                                      \
        assert(!deque_empty(q));              \
        if ((q)->tail > 0) (q)->tail--;       \
        else (q)->tail = (q)->capacity - 1;   \
    } while (0)

#define deque_push_front(q, e)               \
    do {                                     \
        deque_expand(q);                     \
        if ((q)->head > 0) (q)->head--;      \
        else (q)->head = (q)->capacity - 1;  \
        (q)->data[(q)->head] = e;            \
    } while (0)

#define deque_pop_front(q)               \
    do {                                 \
        assert(!deque_empty(q));         \
        (q)->head++;                     \
        if ((q)->head == (q)->capacity)  \
            (q)->head = 0;               \
    } while (0)

#define deque_clear(q)  \
    do {                \
        (q)->head = 0;  \
        (q)->tail = 0;  \
    } while (0)

//       (type, name)
deque_def(int, int);
deque_def(unsigned int, uint);
deque_def(long long, ll);
deque_def(unsigned long long, ull);
deque_def(double, double);
deque_def(char*, str);
deque_def(void*, ptr);

#endif
