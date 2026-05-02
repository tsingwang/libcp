#include "hashset.h"

#include <stdlib.h>

int hashset_init(hashset *set, hashmap_key_kind key_kind, size_t key_size) {
    if (set == NULL) return -1;
    return hashmap_init(&set->map, key_kind, key_size, sizeof(unsigned char),
                        NULL, NULL);
}

void hashset_destroy(hashset *set) {
    if (set == NULL) return;
    hashmap_destroy(&set->map);
}

hashset *hashset_new(hashmap_key_kind key_kind, size_t key_size) {
    hashset *set = malloc(sizeof(*set));
    if (set == NULL) return NULL;
    if (hashset_init(set, key_kind, key_size) != 0) {
        free(set);
        return NULL;
    }
    return set;
}

void hashset_free(hashset *set) {
    if (set == NULL) return;
    hashset_destroy(set);
    free(set);
}

int hashset_copy(hashset *dst, const hashset *src) {
    if (dst == NULL || src == NULL) return -1;
    return hashmap_copy(&dst->map, &src->map);
}

hashset *hashset_clone(const hashset *src) {
    if (src == NULL) return NULL;

    hashset *copy = hashset_new(src->map.key_kind, src->map.key_size);
    if (copy == NULL) return NULL;
    if (hashset_copy(copy, src) != 0) {
        hashset_free(copy);
        return NULL;
    }

    return copy;
}

int hashset_reserve(hashset *set, size_t capacity) {
    if (set == NULL) return -1;
    return hashmap_reserve(&set->map, capacity);
}

int hashset_add(hashset *set, const void *key) {
    static const unsigned char present = 1;
    if (set == NULL) return -1;
    return hashmap_set(&set->map, key, &present);
}

int hashset_contains(const hashset *set, const void *key) {
    if (set == NULL) return 0;
    return hashmap_contains(&set->map, key);
}

int hashset_delete(hashset *set, const void *key) {
    if (set == NULL) return -1;
    return hashmap_delete(&set->map, key);
}

void hashset_clear(hashset *set) {
    if (set == NULL) return;
    hashmap_clear(&set->map);
}

size_t hashset_first(const hashset *set) {
    if (set == NULL) return HASHSET_NPOS;
    return hashmap_first(&set->map);
}

size_t hashset_next(const hashset *set, size_t index) {
    if (set == NULL) return HASHSET_NPOS;
    return hashmap_next(&set->map, index);
}

void *hashset_entry_key(hashset *set, size_t index) {
    if (set == NULL) return NULL;
    return hashmap_entry_key(&set->map, index);
}

const void *hashset_entry_key_const(const hashset *set, size_t index) {
    if (set == NULL) return NULL;
    return hashmap_entry_key_const(&set->map, index);
}
