#include "list.h"

#include <stdlib.h>
#include <string.h>

static int list_copy_element(const list *l, void *dst, const void *src) {
    if (l->elem_copy != NULL) {
        return l->elem_copy(dst, src);
    }

    memcpy(dst, src, l->elem_size);
    return 0;
}

static void list_destroy_element(const list *l, void *elem) {
    if (l->elem_destroy != NULL) {
        l->elem_destroy(elem);
    }
}

static list_node *list_alloc_node(const list *l, const void *elem) {
    if (l == NULL || elem == NULL) return NULL;

    list_node *node = malloc(sizeof(*node) + l->elem_size);
    if (node == NULL) return NULL;

    node->prev = NULL;
    node->next = NULL;
    if (list_copy_element(l, node->data, elem) != 0) {
        free(node);
        return NULL;
    }

    return node;
}

static void list_link_between(list *l, list_node *node,
                              list_node *prev, list_node *next) {
    node->prev = prev;
    node->next = next;
    if (prev != NULL) {
        prev->next = node;
    } else {
        l->head = node;
    }
    if (next != NULL) {
        next->prev = node;
    } else {
        l->tail = node;
    }
    l->size++;
}

static void list_unlink_node(list *l, list_node *node) {
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        l->head = node->next;
    }
    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        l->tail = node->prev;
    }
    l->size--;
}

int list_init(list *l, size_t elem_size,
              list_elem_destroy_fn elem_destroy,
              list_elem_copy_fn elem_copy) {
    if (l == NULL || elem_size == 0) return -1;

    l->size = 0;
    l->elem_size = elem_size;
    l->elem_destroy = elem_destroy;
    l->elem_copy = elem_copy;
    l->head = NULL;
    l->tail = NULL;
    return 0;
}

void list_destroy(list *l) {
    if (l == NULL) return;

    list_clear(l);
    l->head = NULL;
    l->tail = NULL;
}

list *list_new(size_t elem_size,
               list_elem_destroy_fn elem_destroy,
               list_elem_copy_fn elem_copy) {
    list *l = malloc(sizeof(*l));
    if (l == NULL) return NULL;
    if (list_init(l, elem_size, elem_destroy, elem_copy) != 0) {
        free(l);
        return NULL;
    }
    return l;
}

void list_free(list *l) {
    if (l == NULL) return;

    list_destroy(l);
    free(l);
}

int list_copy(list *dst, const list *src) {
    if (dst == NULL || src == NULL) return -1;
    if (dst == src) return 0;

    list_destroy(dst);
    if (list_init(dst, src->elem_size, src->elem_destroy, src->elem_copy) != 0) {
        return -1;
    }

    for (const list_node *node = src->head; node != NULL; node = node->next) {
        if (list_push_back(dst, node->data) != 0) {
            list_clear(dst);
            return -1;
        }
    }

    return 0;
}

list *list_clone(const list *src) {
    if (src == NULL) return NULL;

    list *copy = list_new(src->elem_size, src->elem_destroy, src->elem_copy);
    if (copy == NULL) return NULL;
    if (list_copy(copy, src) != 0) {
        list_free(copy);
        return NULL;
    }

    return copy;
}

int list_push_back(list *l, const void *elem) {
    if (l == NULL || elem == NULL) return -1;

    list_node *node = list_alloc_node(l, elem);
    if (node == NULL) return -1;

    list_link_between(l, node, l->tail, NULL);
    return 0;
}

int list_push_front(list *l, const void *elem) {
    if (l == NULL || elem == NULL) return -1;

    list_node *node = list_alloc_node(l, elem);
    if (node == NULL) return -1;

    list_link_between(l, node, NULL, l->head);
    return 0;
}

int list_pop_back(list *l) {
    if (l == NULL || list_empty(l)) return -1;

    list_erase_node(l, l->tail);
    return 0;
}

int list_pop_front(list *l) {
    if (l == NULL || list_empty(l)) return -1;

    list_erase_node(l, l->head);
    return 0;
}

int list_insert(list *l, size_t index, const void *elem) {
    if (l == NULL || elem == NULL || index > l->size) return -1;
    if (index == 0) return list_push_front(l, elem);
    if (index == l->size) return list_push_back(l, elem);

    list_node *node = list_node_at(l, index);
    if (node == NULL) return -1;
    return list_insert_before(l, node, elem);
}

int list_insert_before(list *l, list_node *node, const void *elem) {
    if (l == NULL || node == NULL || elem == NULL) return -1;

    list_node *new_node = list_alloc_node(l, elem);
    if (new_node == NULL) return -1;

    list_link_between(l, new_node, node->prev, node);
    return 0;
}

int list_insert_after(list *l, list_node *node, const void *elem) {
    if (l == NULL || node == NULL || elem == NULL) return -1;

    list_node *new_node = list_alloc_node(l, elem);
    if (new_node == NULL) return -1;

    list_link_between(l, new_node, node, node->next);
    return 0;
}

int list_set(list *l, size_t index, const void *elem) {
    list_node *node = list_node_at(l, index);
    if (node == NULL) return -1;
    return list_set_node(l, node, elem);
}

int list_set_node(list *l, list_node *node, const void *elem) {
    if (l == NULL || node == NULL || elem == NULL) return -1;

    unsigned char *replacement = malloc(l->elem_size);
    if (replacement == NULL) return -1;
    if (list_copy_element(l, replacement, elem) != 0) {
        free(replacement);
        return -1;
    }

    list_destroy_element(l, node->data);
    memcpy(node->data, replacement, l->elem_size);
    free(replacement);
    return 0;
}

void list_erase(list *l, size_t index) {
    list_node *node = list_node_at(l, index);
    if (node == NULL) return;
    list_erase_node(l, node);
}

void list_erase_node(list *l, list_node *node) {
    if (l == NULL || node == NULL) return;

    list_unlink_node(l, node);
    list_destroy_element(l, node->data);
    free(node);
}

void list_clear(list *l) {
    if (l == NULL) return;

    list_node *node = l->head;
    while (node != NULL) {
        list_node *next = node->next;
        list_destroy_element(l, node->data);
        free(node);
        node = next;
    }

    l->size = 0;
    l->head = NULL;
    l->tail = NULL;
}

list_node *list_front_node(list *l) {
    if (l == NULL) return NULL;
    return l->head;
}

const list_node *list_front_node_const(const list *l) {
    if (l == NULL) return NULL;
    return l->head;
}

list_node *list_back_node(list *l) {
    if (l == NULL) return NULL;
    return l->tail;
}

const list_node *list_back_node_const(const list *l) {
    if (l == NULL) return NULL;
    return l->tail;
}

list_node *list_node_at(list *l, size_t index) {
    if (l == NULL || index >= l->size) return NULL;

    if (index <= l->size / 2) {
        list_node *node = l->head;
        for (size_t i = 0; i < index; i++) {
            node = node->next;
        }
        return node;
    }

    list_node *node = l->tail;
    for (size_t i = l->size - 1; i > index; i--) {
        node = node->prev;
    }
    return node;
}

const list_node *list_node_at_const(const list *l, size_t index) {
    if (l == NULL || index >= l->size) return NULL;

    if (index <= l->size / 2) {
        const list_node *node = l->head;
        for (size_t i = 0; i < index; i++) {
            node = node->next;
        }
        return node;
    }

    const list_node *node = l->tail;
    for (size_t i = l->size - 1; i > index; i--) {
        node = node->prev;
    }
    return node;
}

void *list_node_value(list_node *node) {
    if (node == NULL) return NULL;
    return node->data;
}

const void *list_node_value_const(const list_node *node) {
    if (node == NULL) return NULL;
    return node->data;
}

void *list_at(list *l, size_t index) {
    return list_node_value(list_node_at(l, index));
}

const void *list_at_const(const list *l, size_t index) {
    return list_node_value_const(list_node_at_const(l, index));
}

void *list_front(list *l) {
    return list_node_value(list_front_node(l));
}

const void *list_front_const(const list *l) {
    return list_node_value_const(list_front_node_const(l));
}

void *list_back(list *l) {
    return list_node_value(list_back_node(l));
}

const void *list_back_const(const list *l) {
    return list_node_value_const(list_back_node_const(l));
}
