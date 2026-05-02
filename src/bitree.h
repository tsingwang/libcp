#ifndef __BITREE_H
#define __BITREE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t size;      /* logical element count: valid indices are [0, size) */
    int64_t *nodes;   /* 1-based internal node array of length size + 1 */
} bitree;

#define bitree_size(bt) ((bt)->size)

/* Internal 1-based layout:
 *
 *   nodes[0] is unused
 *   nodes[1] covers [0, 1)
 *   nodes[2] covers [0, 2)
 *   nodes[3] covers [2, 3)
 *   nodes[4] covers [0, 4)
 *   nodes[5] covers [4, 5)
 *   nodes[6] covers [4, 6)
 *   nodes[7] covers [6, 7)
 *   nodes[8] covers [0, 8)
 *
 * Each nodes[i] stores the sum of the last bitree_lowbit(i) values ending at
 * i in 1-based indexing, which maps to a half-open [begin, end) range in the
 * library's 0-based API.
 *
 * Example:
 *   bitree_add(bt, 0, 3);
 *   bitree_add(bt, 2, 5);
 *   bitree_range(bt, 0, 3) == 8;
 */

/* Stack usage:
 *   bitree bt;
 *   bitree_init(&bt, 8);
 *   bitree_destroy(&bt);
 */
int bitree_init(bitree *bt, size_t size);
void bitree_destroy(bitree *bt);

/* Heap usage:
 *   bitree *bt = bitree_new(8);
 *   bitree_free(bt);
 */
bitree *bitree_new(size_t size);
void bitree_free(bitree *bt);

/* copy(...) copies into an already initialized destination. */
int bitree_copy(bitree *dst, const bitree *src);
bitree *bitree_clone(const bitree *src);

/* reset(...) clears all values back to 0. */
int bitree_reset(bitree *bt);

/* add(...) applies a point delta. set(...) overwrites one value. */
int bitree_add(bitree *bt, size_t index, int64_t delta);
int bitree_set(bitree *bt, size_t index, int64_t value);

/* get(...) returns the value at one index. prefix(...) returns the sum of
 * [0, count). range(...) returns [begin, end). All three assert on invalid
 * input.
 */
int64_t bitree_get(const bitree *bt, size_t index);
int64_t bitree_prefix(const bitree *bt, size_t count);
int64_t bitree_range(const bitree *bt, size_t begin, size_t end);

#endif
