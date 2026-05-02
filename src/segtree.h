#ifndef __SEGTREE_H
#define __SEGTREE_H

#include <stddef.h>

typedef int (*segtree_merge_fn)(void *dst, const void *lhs, const void *rhs);

typedef struct {
    size_t size;        /* logical element count: valid indices are [0, size) */
    size_t capacity;    /* internal leaf-layer width and leaf base index:
                         * next power of two >= max(size, 1). It is not the
                         * logical element capacity, and internal-node count is
                         * capacity - 1 rather than capacity.
                         */
    size_t elem_size;   /* bytes per stored value */
    segtree_merge_fn merge;
    unsigned char *identity; /* one copied merge identity (neutral element):
                              * merge(identity, x) == x and merge(x, identity) == x.
                              * It is also used for reset() and empty-range query.
                              */
    unsigned char *nodes;    /* 1-based internal node array of length 2 * capacity */
} segtree;

#define segtree_size(st) ((st)->size)
#define segtree_capacity(st) ((st)->capacity)

/* Example:
 *   segtree_set_value(st, 1, int, 7);
 */
#define segtree_set_value(st, index, T, value) \
    segtree_set((st), (index), &(T){ value })

/* Internal 1-based layout:
 *
 *   nodes[1] is the root
 *   leaves live at nodes[capacity ... capacity + size)
 *   nodes[capacity ... 2 * capacity) is the whole leaf layer
 *   each internal node stores merge(left_child, right_child)
 *
 * query(begin, end, out) uses a half-open [begin, end) range and preserves
 * left-to-right merge order, so it also works for non-commutative merges.
 */

/* Stack usage:
 *   int zero = 0;
 *   segtree st;
 *   segtree_init(&st, 8, sizeof(int), &zero, sum_merge);
 *   segtree_destroy(&st);
 */
int segtree_init(segtree *st, size_t size, size_t elem_size,
                 const void *identity, segtree_merge_fn merge);
void segtree_destroy(segtree *st);

/* Heap usage:
 *   segtree *st = segtree_new(8, sizeof(int), &zero, sum_merge);
 *   segtree_free(st);
 */
segtree *segtree_new(size_t size, size_t elem_size,
                     const void *identity, segtree_merge_fn merge);
void segtree_free(segtree *st);

/* copy(...) copies into an already initialized destination. */
int segtree_copy(segtree *dst, const segtree *src);
segtree *segtree_clone(const segtree *src);

/* reset(...) fills every position back to the identity value. */
int segtree_reset(segtree *st);

/* build(...) loads size contiguous values from elems[0 .. size). */
int segtree_build(segtree *st, const void *elems);

/* set(...) overwrites one value. get(...) copies one value into out. */
int segtree_set(segtree *st, size_t index, const void *elem);
int segtree_get(const segtree *st, size_t index, void *out);

/* query(...) copies merge(values[begin .. end)) into out. It returns the
 * identity value when begin == end.
 */
int segtree_query(const segtree *st, size_t begin, size_t end, void *out);

#endif
