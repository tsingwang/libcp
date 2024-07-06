#ifndef __DICT_H
#define __DICT_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* free expected ‘void *’ when key or value is scalar */
#pragma GCC diagnostic ignored "-Wint-conversion"

#define DICT_INIT_EXP 3
#define DICT_MAX_LOAD_FACTOR 0.75

enum dict_t { DICT_HASH_SET, DICT_HASH_MAP };

/* 0 represents an empty bucket, and the actual 0 value is stored at -1 position */
#define dict_def(name, K, V)                    \
    typedef struct {                            \
        size_t size;                            \
        char exp[2]; /* exponent of capacity */ \
        bool zero_occupied;                     \
        enum dict_t dict_type:1;                \
        int (*key_cmp_func)(const K, const K);  \
        void (*key_free_func)(void*);           \
        void (*value_free_func)(void*);         \
        K* keys[2];                             \
        V* values[2];                           \
        uint32_t* hashes[2];                    \
        int cursor; /* current bucket */        \
    } dict_##name

#define dict_is_hashset(d) ((d)->dict_type == DICT_HASH_SET)
#define dict_is_hashmap(d) ((d)->dict_type == DICT_HASH_MAP)

#define dict_hashes_used(d) _Generic((*(d)->keys[0]), \
        char*: true, \
        default: false)

#define dict_found(d) (((d)->cursor >= 0 && dict_cur_key(d)) \
        || ((d)->cursor == -1 && (d)->zero_occupied))
#define dict_cur_key(d) ((d)->keys[0][(d)->cursor])
#define dict_cur_value(d) ((d)->values[0][(d)->cursor])
#define dict_cur_hash(d) ((d)->hashes[0][(d)->cursor])

#define dict_key_type_bytes(d) sizeof(*((d)->keys[0]))
#define dict_val_type_bytes(d) sizeof(*((d)->values[0]))
#define dict_empty(d) ((d)->size == 0)
#define dict_size(d) ((d)->size)
#define dict_capacity(d) ((d)->exp[0] == -1 ? 0 : (size_t)1<<((d)->exp[0]))
#define dict_i_cap(d, i) ((d)->exp[(i)] == -1 ? 0 : (size_t)1<<((d)->exp[(i)]))
#define dict_i_mask(d, i) ((d)->exp[(i)] == -1 ? 0 : dict_i_cap(d, i) - 1)

#define dict_new_set(d, cmp, fk)     dict_new(d, DICT_HASH_SET, cmp, fk, NULL)
#define dict_new_map(d, cmp, fk, fv) dict_new(d, DICT_HASH_MAP, cmp, fk, fv)

#define dict_new(d, dt, cmp, fk, fv) \
    calloc(1, sizeof(*(d)));         \
    (d)->dict_type = dt;             \
    (d)->key_cmp_func = cmp;         \
    (d)->key_free_func = fk;         \
    (d)->value_free_func = fv;       \
    (d)->exp[0] = -1;                \
    (d)->exp[1] = -1

#define dict_free(d)                                                \
    do {                                                            \
        if ((d) == NULL) return;                                    \
        if ((d)->keys[0]) {                                         \
            if ((d)->key_free_func) {                               \
                for (int i = -1; i < dict_capacity(d); i++) {       \
                    if ((d)->keys[0][i]) {                          \
                        (d)->key_free_func((d)->keys[0][i]);        \
                    }                                               \
                }                                                   \
            }                                                       \
            if ((d)->value_free_func) {                             \
                for (int i = -1; i < dict_capacity(d); i++) {       \
                    if ((d)->values[0][i]) {                        \
                        (d)->value_free_func((d)->values[0][i]);    \
                    }                                               \
                }                                                   \
            }                                                       \
            free((d)->keys[0] - 1);                                 \
            if (dict_is_hashmap(d)) free((d)->values[0] - 1);       \
            if ((d)->hashes[0]) free((d)->hashes[0] - 1);           \
        }                                                           \
        free(d);                                                    \
    } while (0)

#define _dict_find(d, i, key, key_hash)                                 \
    if (key) {                                                          \
        (d)->cursor = key_hash & dict_i_mask(d, i);                     \
        while (true) {                                                  \
            if ((d)->keys[i][(d)->cursor] == 0) {                       \
                break;                                                  \
            }                                                           \
            if ((d)->hashes[i] == NULL) {                               \
                if ((d)->key_cmp_func((d)->keys[i][(d)->cursor], key) == 0) { \
                    break;                                              \
                }                                                       \
            } else if ((d)->hashes[i][(d)->cursor] == key_hash &&       \
                    (d)->key_cmp_func((d)->keys[i][(d)->cursor], key) == 0) {   \
                break;                                                  \
            }                                                           \
            (d)->cursor = ((d)->cursor + 1) & dict_i_mask(d, i);        \
        }                                                               \
    } else {                                                            \
        (d)->cursor = -1;                                               \
    }

#define dict_expand(d)                                                      \
    do {                                                                    \
        if ((d)->size + 1 < dict_i_cap(d, 0) * DICT_MAX_LOAD_FACTOR) break; \
        (d)->exp[1] = (d)->exp[0] <= 0 ? DICT_INIT_EXP : (d)->exp[0] + 1;   \
        (d)->keys[1] = calloc(1 + dict_i_cap(d, 1), dict_key_type_bytes(d));\
        if ((d)->keys[1] == NULL) exit(1);                                  \
        (d)->keys[1] = (d)->keys[1] + 1;                                    \
        if (dict_is_hashmap(d)) {                                           \
            (d)->values[1] = calloc(1 + dict_i_cap(d, 1), dict_val_type_bytes(d));  \
            if ((d)->values[1] == NULL) exit(1);                            \
            (d)->values[1] = (d)->values[1] + 1;                            \
        }                                                                   \
        if (dict_hashes_used(d)) {                                          \
            (d)->hashes[1] = calloc(1 + dict_i_cap(d, 1), sizeof(uint32_t));\
            if ((d)->hashes[1] == NULL) exit(1);                            \
            (d)->hashes[1] = (d)->hashes[1] + 1;                            \
        }                                                                   \
        if ((d)->zero_occupied) { /* copy actual 0 value */                 \
            (d)->keys[1][-1] = (d)->keys[0][-1];                            \
            if ((d)->values[1]) (d)->values[1][-1] = (d)->values[0][-1];    \
            if ((d)->hashes[1]) (d)->hashes[1][-1] = (d)->hashes[0][-1];    \
        }                                                                   \
        for (int i = 0; i < dict_capacity(d); i++) {                        \
            if ((d)->keys[0][i] == 0) continue;                             \
            if ((d)->hashes[0] != NULL) {                                   \
                _dict_find(d, 1, (d)->keys[0][i], (d)->hashes[0][i]);       \
            } else {                                                        \
                _dict_find(d, 1, (d)->keys[0][i], hash_function((d)->keys[0][i]));  \
            }                                                               \
            (d)->keys[1][(d)->cursor] = (d)->keys[0][i];                    \
            if ((d)->values[1]) (d)->values[1][(d)->cursor] = (d)->values[0][i]; \
            if ((d)->hashes[1]) (d)->hashes[1][(d)->cursor] = (d)->hashes[0][i]; \
        }                                                                   \
        if ((d)->keys[0]) free((d)->keys[0] - 1);                           \
        if ((d)->values[0]) free((d)->values[0] - 1);                       \
        if ((d)->hashes[0]) free((d)->hashes[0] - 1);                       \
        (d)->keys[0] = (d)->keys[1];                                        \
        (d)->values[0] = (d)->values[1];                                    \
        (d)->hashes[0] = (d)->hashes[1];                                    \
        (d)->exp[0] = (d)->exp[1];                                          \
        (d)->keys[1] = NULL;                                                \
        (d)->values[1] = NULL;                                              \
        (d)->hashes[1] = NULL;                                              \
    } while (0)

#define dict_find(d, key) _dict_find(d, 0, key, hash_function(key))

#define dict_set_key(d, key)                                \
    do {                                                    \
        dict_expand(d);                                     \
        uint32_t _hash = hash_function(key);                \
        _dict_find(d, 0, key, _hash);                       \
        if (dict_found(d)) break;                           \
        (d)->size++;                                        \
        if ((d)->cursor == -1) (d)->zero_occupied = true;   \
        dict_cur_key(d) = key;                              \
        if ((d)->hashes[0]) dict_cur_hash(d) = _hash;       \
    } while (0)

#define dict_set(d, key, value)                             \
    do {                                                    \
        dict_set_key(d, key);                               \
        if ((d)->value_free_func && dict_cur_value(d)) {    \
            (d)->value_free_func(dict_cur_value(d));        \
        }                                                   \
        dict_cur_value(d) = value;                          \
    } while (0)

#define dict_delete(d, key)                             \
    do {                                                \
        uint32_t _hash = hash_function(key);            \
        _dict_find(d, 0, key, _hash);                   \
        if (!dict_found(d)) break;                      \
        if ((d)->key_free_func) {                       \
            (d)->key_free_func(dict_cur_key(d));        \
            dict_cur_key(d) = NULL;                     \
        } else {                                        \
            dict_cur_key(d) = 0;                        \
        }                                               \
        if ((d)->value_free_func) {                     \
            (d)->value_free_func(dict_cur_value(d));    \
            dict_cur_value(d) = NULL;                   \
        }                                               \
        (d)->size--;                                    \
        if ((d)->cursor == -1) {                        \
            (d)->zero_occupied = false;                 \
            break;                                      \
        }                                               \
        /* Or fake delete: place a tombstone in the entry */    \
        int prev = (d)->cursor;                                 \
        int cur = (d)->cursor;                                  \
        while (true) {                                          \
            cur = (cur + 1) & dict_i_mask(d, 0);                \
            if ((d)->keys[0][cur] == 0) break;                  \
            if ((d)->hashes[0]) _hash = (d)->hashes[0][cur];    \
            else _hash = hash_function((d)->keys[0][cur]);      \
            int p = _hash & dict_i_mask(d, 0);                  \
            if ((cur > prev && p <= prev) || (p > cur && p <= prev)) {          \
                (d)->keys[0][prev] = (d)->keys[0][cur];                         \
                if ((d)->values[0]) (d)->values[0][prev] = (d)->values[0][cur]; \
                if ((d)->hashes[0]) (d)->hashes[0][prev] = (d)->hashes[0][cur]; \
                (d)->keys[0][cur] = 0;                          \
                prev = cur;                                     \
            }                                                   \
        }                                                       \
    } while (0)

#define hash_function(x) _Generic((x), \
        char*: djb2_hash, \
        default: scalar_hash)(x)

static inline uint32_t scalar_hash(uint64_t n) {
    return (uint32_t)n ^ (uint32_t)(n >> 32u);
}

static inline uint32_t djb2_hash(const char *s) {
    uint32_t hash = 5381;
    while (*s) {
        hash = ((hash << 5) + hash) + *s++; // hash * 33 + c
    }
    return hash;
}

//      (name, K,     V)
dict_def(int,  int,   int);
dict_def(ints, int,   char*);
dict_def(str,  char*, char*);
dict_def(sint, char*, int);

#endif
