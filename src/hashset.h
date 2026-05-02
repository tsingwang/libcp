#ifndef __HASHSET_H
#define __HASHSET_H

#include "hashmap.h"

typedef struct {
    hashmap map;
} hashset;

#define HASHSET_NPOS HASHMAP_NPOS
#define hashset_empty(set) hashmap_empty(&(set)->map)
#define hashset_size(set) hashmap_size(&(set)->map)
#define hashset_capacity(set) hashmap_capacity(&(set)->map)

/* Example:
 *   hashset_add_value(set, int, 3);
 *   hashset_contains_value(set, int, 3) == 1;
 */
#define hashset_add_value(set, T, value) hashset_add((set), &(T){ value })
#define hashset_contains_value(set, T, value) hashset_contains((set), &(T){ value })
#define hashset_delete_value(set, T, value) hashset_delete((set), &(T){ value })

/* Stack usage:
 *   hashset set;
 *   hashset_init(&set, HASHMAP_KEY_SCALAR, sizeof(int));
 *   hashset_destroy(&set);
 */
int hashset_init(hashset *set, hashmap_key_kind key_kind, size_t key_size);
void hashset_destroy(hashset *set);

/* Heap usage:
 *   hashset *set = hashset_new(HASHMAP_KEY_SCALAR, sizeof(int));
 *   hashset_free(set);
 */
hashset *hashset_new(hashmap_key_kind key_kind, size_t key_size);
void hashset_free(hashset *set);

int hashset_copy(hashset *dst, const hashset *src);
hashset *hashset_clone(const hashset *src);

int hashset_reserve(hashset *set, size_t capacity);
int hashset_add(hashset *set, const void *key);
int hashset_contains(const hashset *set, const void *key);
int hashset_delete(hashset *set, const void *key);
void hashset_clear(hashset *set);

size_t hashset_first(const hashset *set);
size_t hashset_next(const hashset *set, size_t index);
void *hashset_entry_key(hashset *set, size_t index);
const void *hashset_entry_key_const(const hashset *set, size_t index);

#endif
