#include "segtree.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static size_t segtree_node_count(const segtree *st) {
    return st->capacity * 2;
}

static unsigned char *segtree_node(segtree *st, size_t index) {
    return st->nodes + index * st->elem_size;
}

static const unsigned char *segtree_node_const(const segtree *st, size_t index) {
    return st->nodes + index * st->elem_size;
}

static int segtree_next_capacity(size_t size, size_t *capacity) {
    size_t value = 1;

    while (value < size) {
        if (value > SIZE_MAX / 2) return -1;
        value *= 2;
    }

    *capacity = value;
    return 0;
}

static int segtree_rebuild_node(segtree *st, size_t index) {
    return st->merge(segtree_node(st, index),
                     segtree_node_const(st, index * 2),
                     segtree_node_const(st, index * 2 + 1));
}

int segtree_init(segtree *st, size_t size, size_t elem_size,
                 const void *identity, segtree_merge_fn merge) {
    size_t capacity;
    size_t node_count;

    if (st == NULL || elem_size == 0 || identity == NULL || merge == NULL) {
        return -1;
    }
    if (segtree_next_capacity(size > 0 ? size : 1, &capacity) != 0) return -1;
    if (capacity > SIZE_MAX / 2) return -1;

    st->size = size;
    st->capacity = capacity;
    st->elem_size = elem_size;
    st->merge = merge;
    st->identity = malloc(elem_size);
    if (st->identity == NULL) {
        st->size = 0;
        st->capacity = 0;
        st->elem_size = 0;
        st->merge = NULL;
        st->nodes = NULL;
        return -1;
    }

    memcpy(st->identity, identity, elem_size);
    node_count = capacity * 2;
    st->nodes = malloc(node_count * elem_size);
    if (st->nodes == NULL) {
        free(st->identity);
        st->size = 0;
        st->capacity = 0;
        st->elem_size = 0;
        st->merge = NULL;
        st->identity = NULL;
        return -1;
    }

    for (size_t i = 0; i < node_count; i++) {
        memcpy(segtree_node(st, i), st->identity, elem_size);
    }

    return 0;
}

void segtree_destroy(segtree *st) {
    if (st == NULL) return;

    free(st->identity);
    free(st->nodes);
    st->size = 0;
    st->capacity = 0;
    st->elem_size = 0;
    st->merge = NULL;
    st->identity = NULL;
    st->nodes = NULL;
}

segtree *segtree_new(size_t size, size_t elem_size,
                     const void *identity, segtree_merge_fn merge) {
    segtree *st = malloc(sizeof(*st));

    if (st == NULL) return NULL;
    if (segtree_init(st, size, elem_size, identity, merge) != 0) {
        free(st);
        return NULL;
    }

    return st;
}

void segtree_free(segtree *st) {
    if (st == NULL) return;

    segtree_destroy(st);
    free(st);
}

int segtree_copy(segtree *dst, const segtree *src) {
    if (dst == NULL || src == NULL) return -1;
    if (dst == src) return 0;

    segtree_destroy(dst);
    if (segtree_init(dst, src->size, src->elem_size,
                     src->identity, src->merge) != 0) {
        return -1;
    }

    memcpy(dst->nodes, src->nodes,
           segtree_node_count(src) * src->elem_size);
    return 0;
}

segtree *segtree_clone(const segtree *src) {
    segtree *copy;

    if (src == NULL) return NULL;

    copy = segtree_new(src->size, src->elem_size, src->identity, src->merge);
    if (copy == NULL) return NULL;
    if (segtree_copy(copy, src) != 0) {
        segtree_free(copy);
        return NULL;
    }

    return copy;
}

int segtree_reset(segtree *st) {
    if (st == NULL || st->identity == NULL || st->nodes == NULL) return -1;

    for (size_t i = 0; i < segtree_node_count(st); i++) {
        memcpy(segtree_node(st, i), st->identity, st->elem_size);
    }

    return 0;
}

int segtree_build(segtree *st, const void *elems) {
    const unsigned char *values = elems;

    if (st == NULL) return -1;
    if (st->size > 0 && elems == NULL) return -1;
    if (segtree_reset(st) != 0) return -1;

    for (size_t i = 0; i < st->size; i++) {
        memcpy(segtree_node(st, st->capacity + i),
               values + i * st->elem_size,
               st->elem_size);
    }

    for (size_t i = st->capacity; i-- > 1;) {
        if (segtree_rebuild_node(st, i) != 0) return -1;
    }

    return 0;
}

int segtree_set(segtree *st, size_t index, const void *elem) {
    size_t node_index;

    if (st == NULL || elem == NULL || index >= st->size) return -1;

    node_index = st->capacity + index;
    memcpy(segtree_node(st, node_index), elem, st->elem_size);

    while (node_index > 1) {
        node_index /= 2;
        if (segtree_rebuild_node(st, node_index) != 0) return -1;
    }

    return 0;
}

int segtree_get(const segtree *st, size_t index, void *out) {
    if (st == NULL || out == NULL || index >= st->size) return -1;

    memcpy(out, segtree_node_const(st, st->capacity + index), st->elem_size);
    return 0;
}

int segtree_query(const segtree *st, size_t begin, size_t end, void *out) {
    unsigned char *left_value;
    unsigned char *right_value;
    unsigned char *scratch;
    size_t left;
    size_t right;

    if (st == NULL || out == NULL || begin > end || end > st->size) return -1;
    if (begin == end) {
        memcpy(out, st->identity, st->elem_size);
        return 0;
    }

    left_value = malloc(st->elem_size);
    right_value = malloc(st->elem_size);
    scratch = malloc(st->elem_size);
    if (left_value == NULL || right_value == NULL || scratch == NULL) {
        free(left_value);
        free(right_value);
        free(scratch);
        return -1;
    }

    memcpy(left_value, st->identity, st->elem_size);
    memcpy(right_value, st->identity, st->elem_size);
    left = begin + st->capacity;
    right = end + st->capacity;

    while (left < right) {
        if ((left & 1) != 0) {
            if (st->merge(scratch, left_value, segtree_node_const(st, left)) != 0) {
                free(left_value);
                free(right_value);
                free(scratch);
                return -1;
            }
            memcpy(left_value, scratch, st->elem_size);
            left++;
        }
        if ((right & 1) != 0) {
            right--;
            if (st->merge(scratch, segtree_node_const(st, right), right_value) != 0) {
                free(left_value);
                free(right_value);
                free(scratch);
                return -1;
            }
            memcpy(right_value, scratch, st->elem_size);
        }
        left /= 2;
        right /= 2;
    }

    if (st->merge(out, left_value, right_value) != 0) {
        free(left_value);
        free(right_value);
        free(scratch);
        return -1;
    }

    free(left_value);
    free(right_value);
    free(scratch);
    return 0;
}
