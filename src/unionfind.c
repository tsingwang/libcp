#include "unionfind.h"

#include <stdlib.h>
#include <string.h>

static int unionfind_resize(unionfind *uf, size_t size) {
    size_t *new_parents = NULL;
    size_t *new_sizes = NULL;

    if (size != 0) {
        new_parents = malloc(size * sizeof(*new_parents));
        if (new_parents == NULL) return -1;

        new_sizes = malloc(size * sizeof(*new_sizes));
        if (new_sizes == NULL) {
            free(new_parents);
            return -1;
        }
    }

    free(uf->parents);
    free(uf->sizes);
    uf->parents = new_parents;
    uf->sizes = new_sizes;
    uf->size = size;
    uf->set_count = size;
    return 0;
}

int unionfind_init(unionfind *uf, size_t size) {
    if (uf == NULL) return -1;

    uf->size = 0;
    uf->set_count = 0;
    uf->parents = NULL;
    uf->sizes = NULL;
    if (unionfind_resize(uf, size) != 0) return -1;
    return unionfind_reset(uf);
}

void unionfind_destroy(unionfind *uf) {
    if (uf == NULL) return;

    free(uf->parents);
    free(uf->sizes);
    uf->size = 0;
    uf->set_count = 0;
    uf->parents = NULL;
    uf->sizes = NULL;
}

unionfind *unionfind_new(size_t size) {
    unionfind *uf = malloc(sizeof(*uf));

    if (uf == NULL) return NULL;
    if (unionfind_init(uf, size) != 0) {
        free(uf);
        return NULL;
    }

    return uf;
}

void unionfind_free(unionfind *uf) {
    if (uf == NULL) return;

    unionfind_destroy(uf);
    free(uf);
}

int unionfind_copy(unionfind *dst, const unionfind *src) {
    if (dst == NULL || src == NULL) return -1;
    if (dst == src) return 0;

    if (unionfind_resize(dst, src->size) != 0) return -1;
    if (src->size != 0) {
        memcpy(dst->parents, src->parents, src->size * sizeof(*src->parents));
        memcpy(dst->sizes, src->sizes, src->size * sizeof(*src->sizes));
    }
    dst->set_count = src->set_count;
    return 0;
}

unionfind *unionfind_clone(const unionfind *src) {
    if (src == NULL) return NULL;

    unionfind *copy = unionfind_new(src->size);
    if (copy == NULL) return NULL;
    if (unionfind_copy(copy, src) != 0) {
        unionfind_free(copy);
        return NULL;
    }

    return copy;
}

int unionfind_reset(unionfind *uf) {
    if (uf == NULL) return -1;

    for (size_t i = 0; i < uf->size; ++i) {
        uf->parents[i] = i;
        uf->sizes[i] = 1;
    }
    uf->set_count = uf->size;
    return 0;
}

size_t unionfind_find(unionfind *uf, size_t x) {
    if (uf == NULL || x >= uf->size) return UNIONFIND_INVALID;

    size_t root = x;
    while (uf->parents[root] != root) {
        root = uf->parents[root];
    }

    while (uf->parents[x] != x) {
        size_t parent = uf->parents[x];
        uf->parents[x] = root;
        x = parent;
    }

    return root;
}

size_t unionfind_find_const(const unionfind *uf, size_t x) {
    if (uf == NULL || x >= uf->size) return UNIONFIND_INVALID;

    while (uf->parents[x] != x) {
        x = uf->parents[x];
    }

    return x;
}

int unionfind_union(unionfind *uf, size_t a, size_t b) {
    if (uf == NULL || a >= uf->size || b >= uf->size) return -1;

    size_t root_a = unionfind_find(uf, a);
    size_t root_b = unionfind_find(uf, b);
    if (root_a == root_b) return 0;

    if (uf->sizes[root_a] < uf->sizes[root_b]) {
        size_t tmp = root_a;
        root_a = root_b;
        root_b = tmp;
    }

    uf->parents[root_b] = root_a;
    uf->sizes[root_a] += uf->sizes[root_b];
    uf->sizes[root_b] = 0;
    uf->set_count--;
    return 1;
}

int unionfind_same(unionfind *uf, size_t a, size_t b) {
    if (uf == NULL || a >= uf->size || b >= uf->size) return -1;

    size_t root_a = unionfind_find(uf, a);
    size_t root_b = unionfind_find(uf, b);
    return root_a == root_b;
}

size_t unionfind_set_size(unionfind *uf, size_t x) {
    if (uf == NULL || x >= uf->size) return 0;

    size_t root = unionfind_find(uf, x);
    return uf->sizes[root];
}
