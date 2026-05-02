#include "bitree.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static size_t bitree_lowbit(size_t i) {
    assert(i > 0);
    return i & (~i + 1);
}

static int64_t bitree_prefix_sum(const bitree *bt, size_t count) {
    int64_t sum = 0;

    while (count > 0) {
        sum += bt->nodes[count];
        count -= bitree_lowbit(count);
    }

    return sum;
}

int bitree_init(bitree *bt, size_t size) {
    if (bt == NULL) return -1;

    bt->size = size;
    bt->nodes = calloc(size + 1, sizeof(*bt->nodes));
    if (bt->nodes == NULL) {
        bt->size = 0;
        return -1;
    }

    return 0;
}

void bitree_destroy(bitree *bt) {
    if (bt == NULL) return;

    free(bt->nodes);
    bt->size = 0;
    bt->nodes = NULL;
}

bitree *bitree_new(size_t size) {
    bitree *bt = malloc(sizeof(*bt));

    if (bt == NULL) return NULL;
    if (bitree_init(bt, size) != 0) {
        free(bt);
        return NULL;
    }

    return bt;
}

void bitree_free(bitree *bt) {
    if (bt == NULL) return;

    bitree_destroy(bt);
    free(bt);
}

int bitree_copy(bitree *dst, const bitree *src) {
    if (dst == NULL || src == NULL) return -1;
    if (dst == src) return 0;

    bitree_destroy(dst);
    if (bitree_init(dst, src->size) != 0) return -1;

    memcpy(dst->nodes, src->nodes, (src->size + 1) * sizeof(*src->nodes));
    return 0;
}

bitree *bitree_clone(const bitree *src) {
    if (src == NULL) return NULL;

    bitree *copy = bitree_new(src->size);
    if (copy == NULL) return NULL;
    if (bitree_copy(copy, src) != 0) {
        bitree_free(copy);
        return NULL;
    }

    return copy;
}

int bitree_reset(bitree *bt) {
    if (bt == NULL) return -1;

    memset(bt->nodes, 0, (bt->size + 1) * sizeof(*bt->nodes));
    return 0;
}

int bitree_add(bitree *bt, size_t index, int64_t delta) {
    if (bt == NULL || index >= bt->size) return -1;

    for (size_t i = index + 1; i <= bt->size; i += bitree_lowbit(i)) {
        bt->nodes[i] += delta;
    }

    return 0;
}

int bitree_set(bitree *bt, size_t index, int64_t value) {
    if (bt == NULL || index >= bt->size) return -1;

    int64_t current = bitree_get(bt, index);
    return bitree_add(bt, index, value - current);
}

int64_t bitree_get(const bitree *bt, size_t index) {
    return bitree_range(bt, index, index + 1);
}

int64_t bitree_prefix(const bitree *bt, size_t count) {
    assert(bt != NULL);
    assert(count <= bt->size);
    return bitree_prefix_sum(bt, count);
}

int64_t bitree_range(const bitree *bt, size_t begin, size_t end) {
    assert(bt != NULL);
    assert(begin <= end);
    assert(end <= bt->size);
    return bitree_prefix_sum(bt, end) - bitree_prefix_sum(bt, begin);
}
