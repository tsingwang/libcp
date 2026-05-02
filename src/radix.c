#include "radix.h"

#include <stdlib.h>

int radix_init(radix *set) {
    if (set == NULL) return -1;
    return radixmap_init(&set->map, sizeof(unsigned char), NULL, NULL);
}

void radix_destroy(radix *set) {
    if (set == NULL) return;
    radixmap_destroy(&set->map);
}

radix *radix_new(void) {
    radix *set = malloc(sizeof(*set));
    if (set == NULL) return NULL;
    if (radix_init(set) != 0) {
        free(set);
        return NULL;
    }
    return set;
}

void radix_free(radix *set) {
    if (set == NULL) return;
    radix_destroy(set);
    free(set);
}

int radix_copy(radix *dst, const radix *src) {
    if (dst == NULL || src == NULL) return -1;
    return radixmap_copy(&dst->map, &src->map);
}

radix *radix_clone(const radix *src) {
    if (src == NULL) return NULL;

    radix *copy = radix_new();
    if (copy == NULL) return NULL;
    if (radix_copy(copy, src) != 0) {
        radix_free(copy);
        return NULL;
    }
    return copy;
}

int radix_add(radix *set, const char *key) {
    static const unsigned char present = 1;
    if (set == NULL) return -1;
    return radixmap_set(&set->map, key, &present);
}

int radix_contains(const radix *set, const char *key) {
    if (set == NULL) return 0;
    return radixmap_contains(&set->map, key);
}

int radix_delete(radix *set, const char *key) {
    if (set == NULL) return -1;
    return radixmap_delete(&set->map, key);
}

void radix_clear(radix *set) {
    if (set == NULL) return;
    radixmap_clear(&set->map);
}

int radix_has_prefix(const radix *set, const char *prefix) {
    if (set == NULL) return 0;
    return radixmap_has_prefix(&set->map, prefix);
}
