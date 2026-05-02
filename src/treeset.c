#include "treeset.h"

#include <stdlib.h>

int treeset_init(treeset *set, treemap_key_kind key_kind,
                 size_t key_size, treemap_key_compare_fn key_compare) {
    if (set == NULL) return -1;
    return treemap_init(&set->map, key_kind, key_size, sizeof(unsigned char),
                        key_compare, NULL, NULL);
}

void treeset_destroy(treeset *set) {
    if (set == NULL) return;
    treemap_destroy(&set->map);
}

treeset *treeset_new(treemap_key_kind key_kind, size_t key_size,
                     treemap_key_compare_fn key_compare) {
    treeset *set = malloc(sizeof(*set));
    if (set == NULL) return NULL;
    if (treeset_init(set, key_kind, key_size, key_compare) != 0) {
        free(set);
        return NULL;
    }
    return set;
}

void treeset_free(treeset *set) {
    if (set == NULL) return;
    treeset_destroy(set);
    free(set);
}

int treeset_copy(treeset *dst, const treeset *src) {
    if (dst == NULL || src == NULL) return -1;
    return treemap_copy(&dst->map, &src->map);
}

treeset *treeset_clone(const treeset *src) {
    if (src == NULL) return NULL;

    treeset *copy = treeset_new(src->map.key_kind, src->map.key_size,
                                src->map.key_compare);
    if (copy == NULL) return NULL;
    if (treeset_copy(copy, src) != 0) {
        treeset_free(copy);
        return NULL;
    }

    return copy;
}

int treeset_add(treeset *set, const void *key) {
    static const unsigned char present = 1;
    if (set == NULL) return -1;
    return treemap_set(&set->map, key, &present);
}

int treeset_contains(const treeset *set, const void *key) {
    if (set == NULL) return 0;
    return treemap_contains(&set->map, key);
}

int treeset_delete(treeset *set, const void *key) {
    if (set == NULL) return -1;
    return treemap_delete(&set->map, key);
}

void treeset_clear(treeset *set) {
    if (set == NULL) return;
    treemap_clear(&set->map);
}
