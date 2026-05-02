#ifndef __RADIXMAP_H
#define __RADIXMAP_H

#include <stddef.h>

typedef void (*radixmap_elem_destroy_fn)(void *elem);
typedef int (*radixmap_elem_copy_fn)(void *dst, const void *src);

typedef struct radixmap_node {
    struct radixmap_node *child;        /* first child; descendants continue the key */
    struct radixmap_node *next_sibling; /* next sibling under the same parent */
    char *segment;                      /* compressed edge segment from parent to this node */
    size_t segment_len;                 /* cached strlen(segment) for this compressed edge */
    unsigned char has_value;            /* whether this node itself terminates a stored key */
    unsigned char value[];              /* inline storage for one value when has_value != 0 */
} radixmap_node;

typedef struct {
    size_t size;                           /* number of stored keys */
    size_t value_size;                     /* byte size of each stored value */
    radixmap_elem_destroy_fn value_destroy; /* optional destructor for stored values */
    radixmap_elem_copy_fn value_copy;       /* optional copier for stored values */
    radixmap_node *root;                   /* sentinel root; empty-string keys live here */
} radixmap;

#define radixmap_empty(tree) ((tree)->size == 0)
#define radixmap_size(tree) ((tree)->size)

#define radixmap_get_as(tree, key, T) ((T *)radixmap_at((tree), (key)))
#define radixmap_get_const_as(tree, key, T) ((const T *)radixmap_at_const((tree), (key)))
#define radixmap_contains_value(tree, key) radixmap_contains((tree), (key))
#define radixmap_delete_value(tree, key) radixmap_delete((tree), (key))
#define radixmap_set_value(tree, key, V, value_value) \
    radixmap_set((tree), (key), &(V){ value_value })

/* Stack usage:
 *   radixmap tree;
 *   radixmap_init(&tree, sizeof(int), NULL, NULL);
 *   radixmap_destroy(&tree);
 */
int radixmap_init(radixmap *tree, size_t value_size,
                  radixmap_elem_destroy_fn value_destroy,
                  radixmap_elem_copy_fn value_copy);
void radixmap_destroy(radixmap *tree);

/* Heap usage:
 *   radixmap *tree = radixmap_new(sizeof(int), NULL, NULL);
 *   radixmap_free(tree);
 */
radixmap *radixmap_new(size_t value_size,
                       radixmap_elem_destroy_fn value_destroy,
                       radixmap_elem_copy_fn value_copy);
void radixmap_free(radixmap *tree);

/* copy(...) copies into an already initialized destination. */
int radixmap_copy(radixmap *dst, const radixmap *src);
radixmap *radixmap_clone(const radixmap *src);

int radixmap_set(radixmap *tree, const char *key, const void *value);
int radixmap_contains(const radixmap *tree, const char *key);
int radixmap_delete(radixmap *tree, const char *key);
void radixmap_clear(radixmap *tree);

/* Accessors return stored value addresses for exact keys. */
void *radixmap_at(radixmap *tree, const char *key);
const void *radixmap_at_const(const radixmap *tree, const char *key);

/* has_prefix(...) reports whether any stored key starts with prefix. */
int radixmap_has_prefix(const radixmap *tree, const char *prefix);

#endif
