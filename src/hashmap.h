#ifndef __HASHMAP_H
#define __HASHMAP_H

#include <stddef.h>

typedef void (*hashmap_elem_destroy_fn)(void *elem);
typedef int (*hashmap_elem_copy_fn)(void *dst, const void *src);

typedef enum {
    HASHMAP_KEY_SCALAR = 0,
    HASHMAP_KEY_CSTR = 1,
} hashmap_key_kind;

typedef struct {
    size_t capacity;
    size_t size;
    /* Deleted buckets stay as tombstones so probing chains remain valid after
     * erase; many tombstones trigger rehash just like occupied buckets do. */
    size_t tombstones;
    size_t key_size;
    size_t value_size;
    hashmap_key_kind key_kind;
    hashmap_elem_destroy_fn value_destroy;
    hashmap_elem_copy_fn value_copy;
    /* states stores EMPTY / OCCUPIED / DELETED for each bucket. */
    unsigned char *states;
    unsigned char *keys;
    unsigned char *values;
} hashmap;

#define HASHMAP_INIT_CAPACITY 8
#define HASHMAP_NPOS ((size_t)-1)

#define hashmap_empty(map) ((map)->size == 0)
#define hashmap_size(map) ((map)->size)
#define hashmap_capacity(map) ((map)->capacity)

/* hashmap_at(...) is the low-level accessor: it returns the stored VALUE
 * address for a key address. hashmap_get_as(...) is the usual typed form:
 *   int key = 1;
 *   hashmap_set_value(map, int, 1, int, 42);
 *   *hashmap_at(map, &key) == *(int *)hashmap_at(map, &key);   // low-level
 *   *hashmap_get_as(map, int, 1, int) == 42;                   // typed helper
 * For char* keys:
 *   *hashmap_get_as(map, char *, "alice", int) == 42;
 */
#define hashmap_get_as(map, K, key_value, T) ((T *)hashmap_at((map), &(K){ key_value }))
#define hashmap_get_const_as(map, K, key_value, T) \
    ((const T *)hashmap_at_const((map), &(K){ key_value }))
#define hashmap_contains_value(map, K, key_value) hashmap_contains((map), &(K){ key_value })
#define hashmap_delete_value(map, K, key_value) hashmap_delete((map), &(K){ key_value })
#define hashmap_set_value(map, K, key_value, V, value_value) \
    hashmap_set((map), &(K){ key_value }, &(V){ value_value })

/* Stack usage:
 *   hashmap map;
 *   hashmap_init(&map, HASHMAP_KEY_SCALAR, sizeof(int), sizeof(int), NULL, NULL);
 *   hashmap_destroy(&map);
 * For HASHMAP_KEY_CSTR, key_size must be sizeof(char *) and keys are copied and
 * compared by string contents.
 */
int hashmap_init(hashmap *map, hashmap_key_kind key_kind,
                 size_t key_size, size_t value_size,
                 hashmap_elem_destroy_fn value_destroy,
                 hashmap_elem_copy_fn value_copy);
void hashmap_destroy(hashmap *map);

/* Heap usage:
 *   hashmap *map = hashmap_new(HASHMAP_KEY_SCALAR, sizeof(int), sizeof(int), NULL, NULL);
 *   hashmap_free(map);
 */
hashmap *hashmap_new(hashmap_key_kind key_kind,
                     size_t key_size, size_t value_size,
                     hashmap_elem_destroy_fn value_destroy,
                     hashmap_elem_copy_fn value_copy);
void hashmap_free(hashmap *map);

/* copy(...) copies into an already initialized destination. */
int hashmap_copy(hashmap *dst, const hashmap *src);
hashmap *hashmap_clone(const hashmap *src);

/* reserve() expects logical entry capacity, not raw bucket count. */
int hashmap_reserve(hashmap *map, size_t capacity);
int hashmap_set(hashmap *map, const void *key, const void *value);
int hashmap_contains(const hashmap *map, const void *key);
int hashmap_delete(hashmap *map, const void *key);
void hashmap_clear(hashmap *map);

/* Accessors return value addresses; use hashmap_get_as(...) for typed access. */
void *hashmap_at(hashmap *map, const void *key);
const void *hashmap_at_const(const hashmap *map, const void *key);

/* Iteration walks occupied buckets only; order is unspecified and unstable. */
size_t hashmap_first(const hashmap *map);
size_t hashmap_next(const hashmap *map, size_t index);
void *hashmap_entry_key(hashmap *map, size_t index);
const void *hashmap_entry_key_const(const hashmap *map, size_t index);
void *hashmap_entry_value(hashmap *map, size_t index);
const void *hashmap_entry_value_const(const hashmap *map, size_t index);

#endif
