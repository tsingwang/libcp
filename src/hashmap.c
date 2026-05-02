#include "hashmap.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
    HASHMAP_ENTRY_EMPTY = 0,
    HASHMAP_ENTRY_OCCUPIED = 1,
    HASHMAP_ENTRY_DELETED = 2,
};

static size_t hashmap_hash_scalar_bytes(const unsigned char *bytes, size_t len) {
    size_t hash = 1469598103934665603ull;
    for (size_t i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ull;
    }
    return hash;
}

static size_t hashmap_hash_cstr_value(const char *s) {
    size_t hash = 5381;
    while (*s) {
        hash = ((hash << 5) + hash) + (unsigned char)*s++;
    }
    return hash;
}

static unsigned char *hashmap_key_slot(const hashmap *map, size_t index) {
    return map->keys + index * map->key_size;
}

static unsigned char *hashmap_value_slot(const hashmap *map, size_t index) {
    return map->values + index * map->value_size;
}

static int hashmap_copy_element(hashmap_elem_copy_fn copy_fn, size_t elem_size,
                                void *dst, const void *src) {
    if (copy_fn != NULL) {
        return copy_fn(dst, src);
    }

    memcpy(dst, src, elem_size);
    return 0;
}

static void hashmap_destroy_element(hashmap_elem_destroy_fn destroy_fn, void *elem) {
    if (destroy_fn != NULL) {
        destroy_fn(elem);
    }
}

static int hashmap_copy_key(const hashmap *map, void *dst, const void *src) {
    if (map->key_kind == HASHMAP_KEY_CSTR) {
        const char *value = *(char *const *)src;
        char *copy = malloc(strlen(value) + 1);
        if (copy == NULL) return -1;
        memcpy(copy, value, strlen(value) + 1);
        *(char **)dst = copy;
        return 0;
    }

    memcpy(dst, src, map->key_size);
    return 0;
}

static void hashmap_destroy_key(const hashmap *map, void *elem) {
    if (map->key_kind == HASHMAP_KEY_CSTR) {
        free(*(char **)elem);
    }
}

static int hashmap_key_is_valid(const hashmap *map, const void *key) {
    if (key == NULL) return 0;
    if (map->key_kind == HASHMAP_KEY_CSTR) {
        return *(char *const *)key != NULL;
    }
    return 1;
}

static size_t hashmap_hash_key(const hashmap *map, const void *key) {
    if (map->key_kind == HASHMAP_KEY_CSTR) {
        return hashmap_hash_cstr_value(*(char *const *)key);
    }
    return hashmap_hash_scalar_bytes(key, map->key_size);
}

static int hashmap_keys_equal(const hashmap *map, const void *a, const void *b) {
    if (map->key_kind == HASHMAP_KEY_CSTR) {
        return strcmp(*(char *const *)a, *(char *const *)b) == 0;
    }
    return memcmp(a, b, map->key_size) == 0;
}

static size_t hashmap_bucket_capacity_for_entries(size_t entry_capacity) {
    size_t buckets = HASHMAP_INIT_CAPACITY;
    if (entry_capacity == 0) return 0;

    while (entry_capacity * 4 > buckets * 3) {
        buckets *= 2;
    }
    return buckets;
}

static size_t hashmap_find_slot_in_raw_table(const hashmap *map,
                                             const unsigned char *states,
                                             const unsigned char *keys,
                                             size_t capacity,
                                             const void *key,
                                             size_t hash) {
    size_t mask = capacity - 1;
    size_t index = hash & mask;

    while (states[index] == HASHMAP_ENTRY_OCCUPIED) {
        const unsigned char *slot = keys + index * map->key_size;
        if (hashmap_keys_equal(map, slot, key)) break;
        index = (index + 1) & mask;
    }

    return index;
}

static size_t hashmap_find_occupied_from(const hashmap *map, size_t start) {
    if (map == NULL || start >= map->capacity) return HASHMAP_NPOS;

    for (size_t i = start; i < map->capacity; i++) {
        if (map->states[i] == HASHMAP_ENTRY_OCCUPIED) return i;
    }

    return HASHMAP_NPOS;
}

static size_t hashmap_find_slot(const hashmap *map, const void *key,
                                size_t hash, int *found) {
    size_t mask = map->capacity - 1;
    size_t index = hash & mask;
    size_t first_deleted = SIZE_MAX;

    while (1) {
        unsigned char state = map->states[index];
        if (state == HASHMAP_ENTRY_EMPTY) {
            *found = 0;
            return first_deleted != SIZE_MAX ? first_deleted : index;
        }

        if (state == HASHMAP_ENTRY_OCCUPIED &&
            hashmap_keys_equal(map, hashmap_key_slot(map, index), key)) {
            *found = 1;
            return index;
        }

        if (state == HASHMAP_ENTRY_DELETED && first_deleted == SIZE_MAX) {
            first_deleted = index;
        }

        index = (index + 1) & mask;
    }
}

static int hashmap_rehash(hashmap *map, size_t new_bucket_capacity) {
    unsigned char *new_states = calloc(new_bucket_capacity, sizeof(*new_states));
    unsigned char *new_keys = malloc(new_bucket_capacity * map->key_size);
    unsigned char *new_values = malloc(new_bucket_capacity * map->value_size);
    if (new_states == NULL || new_keys == NULL || new_values == NULL) {
        free(new_states);
        free(new_keys);
        free(new_values);
        return -1;
    }

    for (size_t i = 0; i < map->capacity; i++) {
        if (map->states[i] != HASHMAP_ENTRY_OCCUPIED) continue;

        unsigned char *old_key = hashmap_key_slot(map, i);
        size_t hash = hashmap_hash_key(map, old_key);
        size_t target = hashmap_find_slot_in_raw_table(
            map, new_states, new_keys, new_bucket_capacity, old_key, hash);
        new_states[target] = HASHMAP_ENTRY_OCCUPIED;
        memcpy(new_keys + target * map->key_size, old_key, map->key_size);
        memcpy(new_values + target * map->value_size,
               hashmap_value_slot(map, i), map->value_size);
    }

    free(map->states);
    free(map->keys);
    free(map->values);
    map->states = new_states;
    map->keys = new_keys;
    map->values = new_values;
    map->capacity = new_bucket_capacity;
    map->tombstones = 0;
    return 0;
}

static int hashmap_prepare_insert(hashmap *map) {
    if (map->capacity == 0) {
        return hashmap_rehash(map, HASHMAP_INIT_CAPACITY);
    }

    if ((map->size + map->tombstones + 1) * 4 <= map->capacity * 3) {
        return 0;
    }

    size_t new_capacity = map->capacity * 2;
    return hashmap_rehash(map, new_capacity);
}

int hashmap_init(hashmap *map, hashmap_key_kind key_kind,
                 size_t key_size, size_t value_size,
                 hashmap_elem_destroy_fn value_destroy,
                 hashmap_elem_copy_fn value_copy) {
    if (map == NULL || key_size == 0 || value_size == 0) {
        return -1;
    }
    if (key_kind != HASHMAP_KEY_SCALAR && key_kind != HASHMAP_KEY_CSTR) {
        return -1;
    }
    if (key_kind == HASHMAP_KEY_CSTR && key_size != sizeof(char *)) {
        return -1;
    }

    map->capacity = 0;
    map->size = 0;
    map->tombstones = 0;
    map->key_size = key_size;
    map->value_size = value_size;
    map->key_kind = key_kind;
    map->value_destroy = value_destroy;
    map->value_copy = value_copy;
    map->states = NULL;
    map->keys = NULL;
    map->values = NULL;
    return 0;
}

void hashmap_destroy(hashmap *map) {
    if (map == NULL) return;

    hashmap_clear(map);
    free(map->states);
    free(map->keys);
    free(map->values);
    map->states = NULL;
    map->keys = NULL;
    map->values = NULL;
    map->capacity = 0;
    map->tombstones = 0;
}

hashmap *hashmap_new(hashmap_key_kind key_kind,
                     size_t key_size, size_t value_size,
                     hashmap_elem_destroy_fn value_destroy,
                     hashmap_elem_copy_fn value_copy) {
    hashmap *map = malloc(sizeof(*map));
    if (map == NULL) return NULL;
    if (hashmap_init(map, key_kind, key_size, value_size,
                     value_destroy, value_copy) != 0) {
        free(map);
        return NULL;
    }
    return map;
}

void hashmap_free(hashmap *map) {
    if (map == NULL) return;

    hashmap_destroy(map);
    free(map);
}

int hashmap_copy(hashmap *dst, const hashmap *src) {
    if (dst == NULL || src == NULL) return -1;
    if (dst == src) return 0;

    hashmap_destroy(dst);
    if (hashmap_init(dst, src->key_kind, src->key_size, src->value_size,
                     src->value_destroy, src->value_copy) != 0) {
        return -1;
    }

    if (hashmap_reserve(dst, src->size) != 0) {
        hashmap_clear(dst);
        return -1;
    }

    for (size_t i = hashmap_first(src); i != HASHMAP_NPOS; i = hashmap_next(src, i)) {
        if (hashmap_set(dst,
                        hashmap_entry_key_const(src, i),
                        hashmap_entry_value_const(src, i)) != 0) {
            hashmap_clear(dst);
            return -1;
        }
    }

    return 0;
}

hashmap *hashmap_clone(const hashmap *src) {
    if (src == NULL) return NULL;

    hashmap *copy = hashmap_new(src->key_kind, src->key_size, src->value_size,
                                src->value_destroy, src->value_copy);
    if (copy == NULL) return NULL;
    if (hashmap_copy(copy, src) != 0) {
        hashmap_free(copy);
        return NULL;
    }

    return copy;
}

int hashmap_reserve(hashmap *map, size_t capacity) {
    if (map == NULL) return -1;
    if (capacity == 0) return 0;

    size_t bucket_capacity = hashmap_bucket_capacity_for_entries(capacity);
    if (bucket_capacity < map->capacity) bucket_capacity = map->capacity;
    if (bucket_capacity == map->capacity && map->tombstones == 0) return 0;
    return hashmap_rehash(map, bucket_capacity);
}

int hashmap_set(hashmap *map, const void *key, const void *value) {
    if (map == NULL || !hashmap_key_is_valid(map, key) || value == NULL) return -1;
    if (hashmap_prepare_insert(map) != 0) return -1;

    int found;
    size_t hash = hashmap_hash_key(map, key);
    size_t index = hashmap_find_slot(map, key, hash, &found);
    if (found) {
        unsigned char *staged_value = malloc(map->value_size);
        if (staged_value == NULL) return -1;
        if (hashmap_copy_element(
                map->value_copy, map->value_size, staged_value, value) != 0) {
            free(staged_value);
            return -1;
        }

        hashmap_destroy_element(map->value_destroy, hashmap_value_slot(map, index));
        memcpy(hashmap_value_slot(map, index), staged_value, map->value_size);
        free(staged_value);
        return 0;
    }

    unsigned char *staged_key = malloc(map->key_size);
    if (staged_key == NULL) return -1;
    if (hashmap_copy_key(map, staged_key, key) != 0) {
        free(staged_key);
        return -1;
    }

    unsigned char *staged_value = malloc(map->value_size);
    if (staged_value == NULL) {
        hashmap_destroy_key(map, staged_key);
        free(staged_key);
        return -1;
    }
    if (hashmap_copy_element(
            map->value_copy, map->value_size, staged_value, value) != 0) {
        hashmap_destroy_key(map, staged_key);
        free(staged_key);
        free(staged_value);
        return -1;
    }

    if (map->states[index] == HASHMAP_ENTRY_DELETED) {
        map->tombstones--;
    }

    memcpy(hashmap_key_slot(map, index), staged_key, map->key_size);
    memcpy(hashmap_value_slot(map, index), staged_value, map->value_size);
    map->states[index] = HASHMAP_ENTRY_OCCUPIED;
    map->size++;

    free(staged_key);
    free(staged_value);
    return 0;
}

int hashmap_contains(const hashmap *map, const void *key) {
    if (map == NULL || !hashmap_key_is_valid(map, key) || map->capacity == 0) return 0;
    int found;
    size_t hash = hashmap_hash_key(map, key);
    size_t index = hashmap_find_slot(map, key, hash, &found);
    (void)index;
    return found;
}

int hashmap_delete(hashmap *map, const void *key) {
    if (map == NULL || !hashmap_key_is_valid(map, key) || map->capacity == 0) return -1;

    int found;
    size_t hash = hashmap_hash_key(map, key);
    size_t index = hashmap_find_slot(map, key, hash, &found);
    if (!found) return -1;

    hashmap_destroy_key(map, hashmap_key_slot(map, index));
    hashmap_destroy_element(map->value_destroy, hashmap_value_slot(map, index));
    map->states[index] = HASHMAP_ENTRY_DELETED;
    map->size--;
    map->tombstones++;

    if (map->size == 0) {
        memset(map->states, HASHMAP_ENTRY_EMPTY, map->capacity);
        map->tombstones = 0;
    }

    return 0;
}

void hashmap_clear(hashmap *map) {
    if (map == NULL) return;

    for (size_t i = 0; i < map->capacity; i++) {
        if (map->states[i] != HASHMAP_ENTRY_OCCUPIED) continue;
        hashmap_destroy_key(map, hashmap_key_slot(map, i));
        hashmap_destroy_element(map->value_destroy, hashmap_value_slot(map, i));
    }

    if (map->states != NULL) {
        memset(map->states, HASHMAP_ENTRY_EMPTY, map->capacity);
    }
    map->size = 0;
    map->tombstones = 0;
}

void *hashmap_at(hashmap *map, const void *key) {
    if (map == NULL || !hashmap_key_is_valid(map, key) ||
        map->capacity == 0) {
        return NULL;
    }

    int found;
    size_t hash = hashmap_hash_key(map, key);
    size_t index = hashmap_find_slot(map, key, hash, &found);
    if (!found) return NULL;
    return hashmap_value_slot(map, index);
}

const void *hashmap_at_const(const hashmap *map, const void *key) {
    if (map == NULL || !hashmap_key_is_valid(map, key) ||
        map->capacity == 0) {
        return NULL;
    }

    int found;
    size_t hash = hashmap_hash_key(map, key);
    size_t index = hashmap_find_slot(map, key, hash, &found);
    if (!found) return NULL;
    return hashmap_value_slot(map, index);
}

size_t hashmap_first(const hashmap *map) {
    return hashmap_find_occupied_from(map, 0);
}

size_t hashmap_next(const hashmap *map, size_t index) {
    if (map == NULL || index >= map->capacity) return HASHMAP_NPOS;
    return hashmap_find_occupied_from(map, index + 1);
}

void *hashmap_entry_key(hashmap *map, size_t index) {
    if (map == NULL || index >= map->capacity ||
        map->states[index] != HASHMAP_ENTRY_OCCUPIED) {
        return NULL;
    }

    return hashmap_key_slot(map, index);
}

const void *hashmap_entry_key_const(const hashmap *map, size_t index) {
    if (map == NULL || index >= map->capacity ||
        map->states[index] != HASHMAP_ENTRY_OCCUPIED) {
        return NULL;
    }

    return hashmap_key_slot(map, index);
}

void *hashmap_entry_value(hashmap *map, size_t index) {
    if (map == NULL || index >= map->capacity ||
        map->states[index] != HASHMAP_ENTRY_OCCUPIED) {
        return NULL;
    }

    return hashmap_value_slot(map, index);
}

const void *hashmap_entry_value_const(const hashmap *map, size_t index) {
    if (map == NULL || index >= map->capacity ||
        map->states[index] != HASHMAP_ENTRY_OCCUPIED) {
        return NULL;
    }

    return hashmap_value_slot(map, index);
}
