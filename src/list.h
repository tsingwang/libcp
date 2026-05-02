#ifndef __LIST_H
#define __LIST_H

#include <stddef.h>

typedef void (*list_elem_destroy_fn)(void *elem);
typedef int (*list_elem_copy_fn)(void *dst, const void *src);

typedef struct list_node {
    struct list_node *prev;
    struct list_node *next;
    unsigned char data[];
} list_node;

typedef struct {
    size_t size;
    size_t elem_size;
    list_elem_destroy_fn elem_destroy;
    list_elem_copy_fn elem_copy;
    list_node *head;
    list_node *tail;
} list;

#define list_empty(l) ((l)->size == 0)
#define list_size(l) ((l)->size)

/* Example:
 *   list_push_back_value(l, int, 1);
 *   list_insert_value(l, 1, int, 2);
 *   *list_get_as(l, 0, int) == 1;
 */
#define list_get_as(l, i, T) ((T *)list_at((l), (i)))
#define list_get_const_as(l, i, T) ((const T *)list_at_const((l), (i)))
#define list_front_as(l, T) ((T *)list_front((l)))
#define list_front_const_as(l, T) ((const T *)list_front_const((l)))
#define list_back_as(l, T) ((T *)list_back((l)))
#define list_back_const_as(l, T) ((const T *)list_back_const((l)))
#define list_node_value_as(node, T) ((T *)list_node_value((node)))
#define list_node_value_const_as(node, T) ((const T *)list_node_value_const((node)))
#define list_push_back_value(l, T, value) list_push_back((l), &(T){ value })
#define list_push_front_value(l, T, value) list_push_front((l), &(T){ value })
#define list_insert_value(l, i, T, value) list_insert((l), (i), &(T){ value })
#define list_insert_before_value(l, node, T, value) \
    list_insert_before((l), (node), &(T){ value })
#define list_insert_after_value(l, node, T, value) \
    list_insert_after((l), (node), &(T){ value })
#define list_set_value(l, i, T, value) list_set((l), (i), &(T){ value })
#define list_set_node_value(l, node, T, value) \
    list_set_node((l), (node), &(T){ value })

/* Stack usage:
 *   list l;
 *   list_init(&l, sizeof(int), NULL, NULL);
 *   list_destroy(&l);
 */
int list_init(list *l, size_t elem_size,
              list_elem_destroy_fn elem_destroy,
              list_elem_copy_fn elem_copy);
void list_destroy(list *l);

/* Heap usage:
 *   list *l = list_new(sizeof(int), NULL, NULL);
 *   list_free(l);
 */
list *list_new(size_t elem_size,
               list_elem_destroy_fn elem_destroy,
               list_elem_copy_fn elem_copy);
void list_free(list *l);

int list_copy(list *dst, const list *src);
list *list_clone(const list *src);

/* Low-level api: deal with the address of element or a located node. */
int list_push_back(list *l, const void *elem);
int list_push_front(list *l, const void *elem);
int list_pop_back(list *l);
int list_pop_front(list *l);
int list_insert(list *l, size_t index, const void *elem);
int list_insert_before(list *l, list_node *node, const void *elem);
int list_insert_after(list *l, list_node *node, const void *elem);
int list_set(list *l, size_t index, const void *elem);
int list_set_node(list *l, list_node *node, const void *elem);
void list_erase(list *l, size_t index);
void list_erase_node(list *l, list_node *node);
void list_clear(list *l);

/* Accessors return element addresses or nodes for stable positioning. */
list_node *list_front_node(list *l);
const list_node *list_front_node_const(const list *l);
list_node *list_back_node(list *l);
const list_node *list_back_node_const(const list *l);
list_node *list_node_at(list *l, size_t index);
const list_node *list_node_at_const(const list *l, size_t index);
void *list_node_value(list_node *node);
const void *list_node_value_const(const list_node *node);
void *list_at(list *l, size_t index);
const void *list_at_const(const list *l, size_t index);
void *list_front(list *l);
const void *list_front_const(const list *l);
void *list_back(list *l);
const void *list_back_const(const list *l);

#endif
