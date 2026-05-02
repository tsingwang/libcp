#ifndef __UNIONFIND_H
#define __UNIONFIND_H

#include <stddef.h>

typedef struct {
    size_t size;       /* fixed element count: valid ids are [0, size) */
    size_t set_count;  /* current disjoint-set count, always 0 <= set_count <= size */
    size_t *parents;   /* forest parent links; a root satisfies parents[x] == x */
    size_t *sizes;     /* root-only: sizes[root] is that set's size */
} unionfind;

#define UNIONFIND_INVALID ((size_t)-1)
#define unionfind_size(uf) ((uf)->size)
#define unionfind_set_count(uf) ((uf)->set_count)

/* Example:
 *   unionfind_union(uf, 0, 1);
 *   unionfind_same(uf, 0, 1) == 1;
 *   unionfind_set_size(uf, 0) == 2;
 */

/* Stack usage:
 *   unionfind uf;
 *   unionfind_init(&uf, 8);
 *   unionfind_destroy(&uf);
 */
int unionfind_init(unionfind *uf, size_t size);
void unionfind_destroy(unionfind *uf);

/* Heap usage:
 *   unionfind *uf = unionfind_new(8);
 *   unionfind_free(uf);
 */
unionfind *unionfind_new(size_t size);
void unionfind_free(unionfind *uf);

/* copy(...) copies into an already initialized destination. */
int unionfind_copy(unionfind *dst, const unionfind *src);
unionfind *unionfind_clone(const unionfind *src);

/* reset(...) restores all elements to singleton sets. */
int unionfind_reset(unionfind *uf);

/* find(...) applies path compression. find_const(...) does not mutate state. */
size_t unionfind_find(unionfind *uf, size_t x);
size_t unionfind_find_const(const unionfind *uf, size_t x);

/* union(...) returns 1 if two sets were merged, 0 if already connected,
 * and -1 on invalid input. */
int unionfind_union(unionfind *uf, size_t a, size_t b);
int unionfind_same(unionfind *uf, size_t a, size_t b);
size_t unionfind_set_size(unionfind *uf, size_t x);

#endif
