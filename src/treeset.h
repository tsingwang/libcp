#ifndef __TREESET_H
#define __TREESET_H

#include "treemap.h"

typedef struct {
    treemap map;
} treeset;

#define treeset_empty(set) treemap_empty(&(set)->map)
#define treeset_size(set) treemap_size(&(set)->map)

#define treeset_add_value(set, K, key_value) treeset_add((set), &(K){ key_value })
#define treeset_contains_value(set, K, key_value) \
    treeset_contains((set), &(K){ key_value })
#define treeset_delete_value(set, K, key_value) treeset_delete((set), &(K){ key_value })

#define treeset_first(set) treemap_first(&(set)->map)
#define treeset_last(set) treemap_last(&(set)->map)
#define treeset_lower_bound(set, key) treemap_lower_bound(&(set)->map, (key))
#define treeset_lower_bound_const(set, key) treemap_lower_bound_const(&(set)->map, (key))
#define treeset_upper_bound(set, key) treemap_upper_bound(&(set)->map, (key))
#define treeset_upper_bound_const(set, key) treemap_upper_bound_const(&(set)->map, (key))
#define treeset_next(node) treemap_next((node))
#define treeset_prev(node) treemap_prev((node))
#define treeset_node_key_as(set, node, T) \
    ((T *)treemap_node_key(&(set)->map, (node)))
#define treeset_node_key_const_as(set, node, T) \
    ((const T *)treemap_node_key_const(&(set)->map, (node)))

/* Stack usage:
 *   treeset set;
 *   treeset_init(&set, TREEMAP_KEY_SCALAR, sizeof(int), int_cmp);
 *   treeset_destroy(&set);
 */
int treeset_init(treeset *set, treemap_key_kind key_kind,
                 size_t key_size, treemap_key_compare_fn key_compare);
void treeset_destroy(treeset *set);

/* Heap usage:
 *   treeset *set = treeset_new(TREEMAP_KEY_SCALAR, sizeof(int), int_cmp);
 *   treeset_free(set);
 */
treeset *treeset_new(treemap_key_kind key_kind, size_t key_size,
                     treemap_key_compare_fn key_compare);
void treeset_free(treeset *set);

int treeset_copy(treeset *dst, const treeset *src);
treeset *treeset_clone(const treeset *src);

int treeset_add(treeset *set, const void *key);
int treeset_contains(const treeset *set, const void *key);
int treeset_delete(treeset *set, const void *key);
void treeset_clear(treeset *set);

#endif
