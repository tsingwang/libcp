#ifndef __TREEMAP_H
#define __TREEMAP_H

#include <stddef.h>

typedef void (*treemap_elem_destroy_fn)(void *elem);
typedef int (*treemap_elem_copy_fn)(void *dst, const void *src);
typedef int (*treemap_key_compare_fn)(const void *a, const void *b);

typedef enum {
    TREEMAP_KEY_SCALAR = 0,
    TREEMAP_KEY_CSTR = 1,
} treemap_key_kind;

typedef enum {
    TREEMAP_RED = 0,
    TREEMAP_BLACK = 1,
} treemap_color;

typedef struct treemap_node {
    struct treemap_node *left;
    struct treemap_node *right;
    struct treemap_node *parent;
    unsigned char color;
    unsigned char data[];
} treemap_node;

typedef struct {
    size_t size;
    size_t key_size;
    size_t value_size;
    treemap_key_kind key_kind;
    treemap_key_compare_fn key_compare;
    treemap_elem_destroy_fn value_destroy;
    treemap_elem_copy_fn value_copy;
    treemap_node *root;
} treemap;

#define treemap_empty(map) ((map)->size == 0)
#define treemap_size(map) ((map)->size)

/* hashmap_get_as(...) returns the VALUE stored for a key. treemap_get_as(...)
 * does the same, but tree ordering also requires key_compare:
 *   treemap_set_value(map, int, 1, int, 42);
 *   *treemap_get_as(map, int, 1, int) == 42;
 *   *treemap_get_as(map, char *, "alice", int) == 42;
 */
#define treemap_get_as(map, K, key_value, T) ((T *)treemap_at((map), &(K){ key_value }))
#define treemap_get_const_as(map, K, key_value, T) \
    ((const T *)treemap_at_const((map), &(K){ key_value }))
#define treemap_contains_value(map, K, key_value) treemap_contains((map), &(K){ key_value })
#define treemap_delete_value(map, K, key_value) treemap_delete((map), &(K){ key_value })
#define treemap_set_value(map, K, key_value, V, value_value) \
    treemap_set((map), &(K){ key_value }, &(V){ value_value })

#define treemap_node_key_as(map, node, T) ((T *)treemap_node_key((map), (node)))
#define treemap_node_key_const_as(map, node, T) \
    ((const T *)treemap_node_key_const((map), (node)))
#define treemap_node_value_as(map, node, T) ((T *)treemap_node_value((map), (node)))
#define treemap_node_value_const_as(map, node, T) \
    ((const T *)treemap_node_value_const((map), (node)))

/* Stack usage:
 *   treemap map;
 *   treemap_init(&map, TREEMAP_KEY_SCALAR, sizeof(int), sizeof(int), int_cmp,
 *                NULL, NULL);
 *   treemap_destroy(&map);
 * For TREEMAP_KEY_CSTR, key_size must be sizeof(char *) and keys are copied
 * and compared by string contents through key_compare.
 */
int treemap_init(treemap *map, treemap_key_kind key_kind,
                 size_t key_size, size_t value_size,
                 treemap_key_compare_fn key_compare,
                 treemap_elem_destroy_fn value_destroy,
                 treemap_elem_copy_fn value_copy);
void treemap_destroy(treemap *map);

/* Heap usage:
 *   treemap *map = treemap_new(TREEMAP_KEY_SCALAR, sizeof(int), sizeof(int),
 *                              int_cmp, NULL, NULL);
 *   treemap_free(map);
 */
treemap *treemap_new(treemap_key_kind key_kind,
                     size_t key_size, size_t value_size,
                     treemap_key_compare_fn key_compare,
                     treemap_elem_destroy_fn value_destroy,
                     treemap_elem_copy_fn value_copy);
void treemap_free(treemap *map);

/* copy(...) copies into an already initialized destination. */
int treemap_copy(treemap *dst, const treemap *src);
treemap *treemap_clone(const treemap *src);

int treemap_set(treemap *map, const void *key, const void *value);
int treemap_contains(const treemap *map, const void *key);
int treemap_delete(treemap *map, const void *key);
void treemap_clear(treemap *map);

/* Accessors return value addresses; use treemap_get_as(...) for typed access. */
void *treemap_at(treemap *map, const void *key);
const void *treemap_at_const(const treemap *map, const void *key);

/* Ordered traversal walks nodes in key order. */
treemap_node *treemap_first(treemap *map);
const treemap_node *treemap_first_const(const treemap *map);
treemap_node *treemap_last(treemap *map);
const treemap_node *treemap_last_const(const treemap *map);
treemap_node *treemap_lower_bound(treemap *map, const void *key);
const treemap_node *treemap_lower_bound_const(const treemap *map, const void *key);
treemap_node *treemap_upper_bound(treemap *map, const void *key);
const treemap_node *treemap_upper_bound_const(const treemap *map, const void *key);
treemap_node *treemap_next(treemap_node *node);
const treemap_node *treemap_next_const(const treemap_node *node);
treemap_node *treemap_prev(treemap_node *node);
const treemap_node *treemap_prev_const(const treemap_node *node);

/* Node accessors expose the key and value stored in one visited node. */
void *treemap_node_key(treemap *map, treemap_node *node);
const void *treemap_node_key_const(const treemap *map, const treemap_node *node);
void *treemap_node_value(treemap *map, treemap_node *node);
const void *treemap_node_value_const(const treemap *map, const treemap_node *node);

#endif
