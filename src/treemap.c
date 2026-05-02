#include "treemap.h"

#include <stdlib.h>
#include <string.h>

static unsigned char *treemap_key_slot(const treemap *map, treemap_node *node) {
    return node->data;
}

static const unsigned char *treemap_key_slot_const(const treemap *map,
                                                   const treemap_node *node) {
    (void)map;
    return node->data;
}

static unsigned char *treemap_value_slot(const treemap *map, treemap_node *node) {
    return node->data + map->key_size;
}

static const unsigned char *treemap_value_slot_const(const treemap *map,
                                                     const treemap_node *node) {
    return node->data + map->key_size;
}

static int treemap_copy_value(const treemap *map, void *dst, const void *src) {
    if (map->value_copy != NULL) {
        return map->value_copy(dst, src);
    }

    memcpy(dst, src, map->value_size);
    return 0;
}

static void treemap_destroy_value(const treemap *map, void *value) {
    if (map->value_destroy != NULL) {
        map->value_destroy(value);
    }
}

static int treemap_copy_key(const treemap *map, void *dst, const void *src) {
    if (map->key_kind == TREEMAP_KEY_CSTR) {
        const char *value = *(char *const *)src;
        char *copy = malloc(strlen(value) + 1);
        if (copy == NULL) return -1;
        memcpy(copy, value, strlen(value) + 1);
        *(char **)dst = copy;
        return 0;
    }

    memcpy(dst, src, map->key_size);
    return 0;
}

static void treemap_destroy_key(const treemap *map, void *key) {
    if (map->key_kind == TREEMAP_KEY_CSTR) {
        free(*(char **)key);
    }
}

static int treemap_key_is_valid(const treemap *map, const void *key) {
    if (key == NULL) return 0;
    if (map->key_kind == TREEMAP_KEY_CSTR) {
        return *(char *const *)key != NULL;
    }
    return 1;
}

static treemap_node *treemap_minimum(treemap_node *node) {
    while (node != NULL && node->left != NULL) {
        node = node->left;
    }
    return node;
}

static treemap_node *treemap_maximum(treemap_node *node) {
    while (node != NULL && node->right != NULL) {
        node = node->right;
    }
    return node;
}

static treemap_color treemap_color_of(const treemap_node *node) {
    return node == NULL ? TREEMAP_BLACK : (treemap_color)node->color;
}

static void treemap_set_color(treemap_node *node, treemap_color color) {
    if (node != NULL) {
        node->color = (unsigned char)color;
    }
}

static treemap_node *treemap_left_child(treemap_node *node) {
    return node == NULL ? NULL : node->left;
}

static treemap_node *treemap_right_child(treemap_node *node) {
    return node == NULL ? NULL : node->right;
}

static treemap_node *treemap_find_node(const treemap *map, const void *key) {
    treemap_node *node = map->root;

    while (node != NULL) {
        int cmp = map->key_compare(treemap_key_slot_const(map, node), key);
        if (cmp > 0) {
            node = node->left;
        } else if (cmp < 0) {
            node = node->right;
        } else {
            return node;
        }
    }

    return NULL;
}

static treemap_node *treemap_find_bound_node(const treemap *map, const void *key,
                                             int strict_greater) {
    treemap_node *node;
    treemap_node *bound = NULL;

    if (map == NULL || !treemap_key_is_valid(map, key)) return NULL;

    node = map->root;
    while (node != NULL) {
        int cmp = map->key_compare(treemap_key_slot_const(map, node), key);
        if (cmp > 0 || (!strict_greater && cmp == 0)) {
            bound = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }

    return bound;
}

static void treemap_left_rotate(treemap *map, treemap_node *node) {
    treemap_node *right = node->right;

    node->right = right->left;
    if (right->left != NULL) {
        right->left->parent = node;
    }

    right->parent = node->parent;
    if (node->parent == NULL) {
        map->root = right;
    } else if (node == node->parent->left) {
        node->parent->left = right;
    } else {
        node->parent->right = right;
    }

    right->left = node;
    node->parent = right;
}

static void treemap_right_rotate(treemap *map, treemap_node *node) {
    treemap_node *left = node->left;

    node->left = left->right;
    if (left->right != NULL) {
        left->right->parent = node;
    }

    left->parent = node->parent;
    if (node->parent == NULL) {
        map->root = left;
    } else if (node == node->parent->right) {
        node->parent->right = left;
    } else {
        node->parent->left = left;
    }

    left->right = node;
    node->parent = left;
}

static void treemap_insert_fixup(treemap *map, treemap_node *node) {
    while (node != map->root && treemap_color_of(node->parent) == TREEMAP_RED) {
        if (node->parent == node->parent->parent->left) {
            treemap_node *uncle = node->parent->parent->right;

            if (treemap_color_of(uncle) == TREEMAP_RED) {
                treemap_set_color(node->parent, TREEMAP_BLACK);
                treemap_set_color(uncle, TREEMAP_BLACK);
                treemap_set_color(node->parent->parent, TREEMAP_RED);
                node = node->parent->parent;
            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    treemap_left_rotate(map, node);
                }
                treemap_set_color(node->parent, TREEMAP_BLACK);
                treemap_set_color(node->parent->parent, TREEMAP_RED);
                treemap_right_rotate(map, node->parent->parent);
            }
        } else {
            treemap_node *uncle = node->parent->parent->left;

            if (treemap_color_of(uncle) == TREEMAP_RED) {
                treemap_set_color(node->parent, TREEMAP_BLACK);
                treemap_set_color(uncle, TREEMAP_BLACK);
                treemap_set_color(node->parent->parent, TREEMAP_RED);
                node = node->parent->parent;
            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    treemap_right_rotate(map, node);
                }
                treemap_set_color(node->parent, TREEMAP_BLACK);
                treemap_set_color(node->parent->parent, TREEMAP_RED);
                treemap_left_rotate(map, node->parent->parent);
            }
        }
    }

    treemap_set_color(map->root, TREEMAP_BLACK);
}

static void treemap_transplant(treemap *map, treemap_node *old_node,
                               treemap_node *new_node) {
    if (old_node->parent == NULL) {
        map->root = new_node;
    } else if (old_node == old_node->parent->left) {
        old_node->parent->left = new_node;
    } else {
        old_node->parent->right = new_node;
    }

    if (new_node != NULL) {
        new_node->parent = old_node->parent;
    }
}

static void treemap_delete_fixup(treemap *map, treemap_node *node,
                                 treemap_node *parent) {
    while (node != map->root && treemap_color_of(node) == TREEMAP_BLACK) {
        treemap_node *sibling;
        treemap_node *sibling_left;
        treemap_node *sibling_right;

        if (parent == NULL) break;

        if (node == parent->left) {
            sibling = parent->right;
            if (treemap_color_of(sibling) == TREEMAP_RED) {
                treemap_set_color(sibling, TREEMAP_BLACK);
                treemap_set_color(parent, TREEMAP_RED);
                treemap_left_rotate(map, parent);
                sibling = parent->right;
            }

            sibling_left = treemap_left_child(sibling);
            sibling_right = treemap_right_child(sibling);
            if (treemap_color_of(sibling_left) == TREEMAP_BLACK &&
                treemap_color_of(sibling_right) == TREEMAP_BLACK) {
                treemap_set_color(sibling, TREEMAP_RED);
                node = parent;
                parent = parent->parent;
            } else {
                if (treemap_color_of(sibling_right) == TREEMAP_BLACK) {
                    treemap_set_color(sibling_left, TREEMAP_BLACK);
                    treemap_set_color(sibling, TREEMAP_RED);
                    if (sibling != NULL) {
                        treemap_right_rotate(map, sibling);
                    }
                    sibling = parent->right;
                    sibling_left = treemap_left_child(sibling);
                    sibling_right = treemap_right_child(sibling);
                }

                treemap_set_color(sibling, treemap_color_of(parent));
                treemap_set_color(parent, TREEMAP_BLACK);
                treemap_set_color(sibling_right, TREEMAP_BLACK);
                treemap_left_rotate(map, parent);
                node = map->root;
                parent = NULL;
            }
        } else {
            sibling = parent->left;
            if (treemap_color_of(sibling) == TREEMAP_RED) {
                treemap_set_color(sibling, TREEMAP_BLACK);
                treemap_set_color(parent, TREEMAP_RED);
                treemap_right_rotate(map, parent);
                sibling = parent->left;
            }

            sibling_left = treemap_left_child(sibling);
            sibling_right = treemap_right_child(sibling);
            if (treemap_color_of(sibling_right) == TREEMAP_BLACK &&
                treemap_color_of(sibling_left) == TREEMAP_BLACK) {
                treemap_set_color(sibling, TREEMAP_RED);
                node = parent;
                parent = parent->parent;
            } else {
                if (treemap_color_of(sibling_left) == TREEMAP_BLACK) {
                    treemap_set_color(sibling_right, TREEMAP_BLACK);
                    treemap_set_color(sibling, TREEMAP_RED);
                    if (sibling != NULL) {
                        treemap_left_rotate(map, sibling);
                    }
                    sibling = parent->left;
                    sibling_left = treemap_left_child(sibling);
                    sibling_right = treemap_right_child(sibling);
                }

                treemap_set_color(sibling, treemap_color_of(parent));
                treemap_set_color(parent, TREEMAP_BLACK);
                treemap_set_color(sibling_left, TREEMAP_BLACK);
                treemap_right_rotate(map, parent);
                node = map->root;
                parent = NULL;
            }
        }
    }

    treemap_set_color(node, TREEMAP_BLACK);
}

static treemap_node *treemap_create_node(const treemap *map,
                                         const void *key, const void *value) {
    treemap_node *node = malloc(sizeof(*node) + map->key_size + map->value_size);
    if (node == NULL) return NULL;

    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->color = (unsigned char)TREEMAP_RED;

    if (treemap_copy_key(map, treemap_key_slot(map, node), key) != 0) {
        free(node);
        return NULL;
    }

    if (treemap_copy_value(map, treemap_value_slot(map, node), value) != 0) {
        treemap_destroy_key(map, treemap_key_slot(map, node));
        free(node);
        return NULL;
    }

    return node;
}

static void treemap_clear_node(treemap *map, treemap_node *node) {
    if (node == NULL) return;

    treemap_clear_node(map, node->left);
    treemap_clear_node(map, node->right);
    treemap_destroy_key(map, treemap_key_slot(map, node));
    treemap_destroy_value(map, treemap_value_slot(map, node));
    free(node);
}

int treemap_init(treemap *map, treemap_key_kind key_kind,
                 size_t key_size, size_t value_size,
                 treemap_key_compare_fn key_compare,
                 treemap_elem_destroy_fn value_destroy,
                 treemap_elem_copy_fn value_copy) {
    if (map == NULL || key_size == 0 || value_size == 0 || key_compare == NULL) {
        return -1;
    }
    if (key_kind != TREEMAP_KEY_SCALAR && key_kind != TREEMAP_KEY_CSTR) {
        return -1;
    }
    if (key_kind == TREEMAP_KEY_CSTR && key_size != sizeof(char *)) {
        return -1;
    }

    map->size = 0;
    map->key_size = key_size;
    map->value_size = value_size;
    map->key_kind = key_kind;
    map->key_compare = key_compare;
    map->value_destroy = value_destroy;
    map->value_copy = value_copy;
    map->root = NULL;
    return 0;
}

void treemap_destroy(treemap *map) {
    if (map == NULL) return;

    treemap_clear(map);
}

treemap *treemap_new(treemap_key_kind key_kind,
                     size_t key_size, size_t value_size,
                     treemap_key_compare_fn key_compare,
                     treemap_elem_destroy_fn value_destroy,
                     treemap_elem_copy_fn value_copy) {
    treemap *map = malloc(sizeof(*map));
    if (map == NULL) return NULL;
    if (treemap_init(map, key_kind, key_size, value_size, key_compare,
                     value_destroy, value_copy) != 0) {
        free(map);
        return NULL;
    }
    return map;
}

void treemap_free(treemap *map) {
    if (map == NULL) return;

    treemap_destroy(map);
    free(map);
}

int treemap_copy(treemap *dst, const treemap *src) {
    if (dst == NULL || src == NULL) return -1;
    if (dst == src) return 0;

    treemap_destroy(dst);
    if (treemap_init(dst, src->key_kind, src->key_size, src->value_size,
                     src->key_compare, src->value_destroy, src->value_copy) != 0) {
        return -1;
    }

    for (const treemap_node *node = treemap_first_const(src); node != NULL;
         node = treemap_next_const(node)) {
        if (treemap_set(dst,
                        treemap_node_key_const(src, node),
                        treemap_node_value_const(src, node)) != 0) {
            treemap_clear(dst);
            return -1;
        }
    }

    return 0;
}

treemap *treemap_clone(const treemap *src) {
    treemap *copy;

    if (src == NULL) return NULL;

    copy = treemap_new(src->key_kind, src->key_size, src->value_size,
                       src->key_compare, src->value_destroy, src->value_copy);
    if (copy == NULL) return NULL;
    if (treemap_copy(copy, src) != 0) {
        treemap_free(copy);
        return NULL;
    }

    return copy;
}

int treemap_set(treemap *map, const void *key, const void *value) {
    treemap_node *node;
    treemap_node *parent = NULL;
    treemap_node *created;
    int cmp = 0;

    if (map == NULL || !treemap_key_is_valid(map, key) || value == NULL) return -1;

    node = map->root;
    while (node != NULL) {
        parent = node;
        cmp = map->key_compare(treemap_key_slot_const(map, node), key);
        if (cmp > 0) {
            node = node->left;
        } else if (cmp < 0) {
            node = node->right;
        } else {
            unsigned char *replacement = malloc(map->value_size);
            if (replacement == NULL) return -1;
            if (treemap_copy_value(map, replacement, value) != 0) {
                free(replacement);
                return -1;
            }
            treemap_destroy_value(map, treemap_value_slot(map, node));
            memcpy(treemap_value_slot(map, node), replacement, map->value_size);
            free(replacement);
            return 0;
        }
    }

    created = treemap_create_node(map, key, value);
    if (created == NULL) return -1;

    created->parent = parent;
    if (parent == NULL) {
        map->root = created;
    } else if (cmp > 0) {
        parent->left = created;
    } else {
        parent->right = created;
    }

    map->size++;
    treemap_insert_fixup(map, created);
    return 0;
}

int treemap_contains(const treemap *map, const void *key) {
    if (map == NULL || !treemap_key_is_valid(map, key)) return 0;
    return treemap_find_node(map, key) != NULL;
}

int treemap_delete(treemap *map, const void *key) {
    treemap_node *node;
    treemap_node *fix_node;
    treemap_node *fix_parent;
    treemap_node *moved;
    treemap_color removed_color;

    if (map == NULL || !treemap_key_is_valid(map, key)) return -1;

    node = treemap_find_node(map, key);
    if (node == NULL) return -1;

    moved = node;
    removed_color = (treemap_color)moved->color;
    if (node->left == NULL) {
        fix_node = node->right;
        fix_parent = node->parent;
        treemap_transplant(map, node, node->right);
    } else if (node->right == NULL) {
        fix_node = node->left;
        fix_parent = node->parent;
        treemap_transplant(map, node, node->left);
    } else {
        moved = treemap_minimum(node->right);
        removed_color = (treemap_color)moved->color;
        fix_node = moved->right;
        if (moved->parent == node) {
            fix_parent = moved;
            if (fix_node != NULL) {
                fix_node->parent = moved;
            }
        } else {
            fix_parent = moved->parent;
            treemap_transplant(map, moved, moved->right);
            moved->right = node->right;
            moved->right->parent = moved;
        }

        treemap_transplant(map, node, moved);
        moved->left = node->left;
        moved->left->parent = moved;
        moved->color = node->color;
    }

    treemap_destroy_key(map, treemap_key_slot(map, node));
    treemap_destroy_value(map, treemap_value_slot(map, node));
    free(node);
    map->size--;

    if (removed_color == TREEMAP_BLACK) {
        treemap_delete_fixup(map, fix_node, fix_parent);
    }

    return 0;
}

void treemap_clear(treemap *map) {
    if (map == NULL) return;

    treemap_clear_node(map, map->root);
    map->root = NULL;
    map->size = 0;
}

void *treemap_at(treemap *map, const void *key) {
    treemap_node *node;

    if (map == NULL || !treemap_key_is_valid(map, key)) return NULL;
    node = treemap_find_node(map, key);
    if (node == NULL) return NULL;
    return treemap_value_slot(map, node);
}

const void *treemap_at_const(const treemap *map, const void *key) {
    treemap_node *node;

    if (map == NULL || !treemap_key_is_valid(map, key)) return NULL;
    node = treemap_find_node(map, key);
    if (node == NULL) return NULL;
    return treemap_value_slot_const(map, node);
}

treemap_node *treemap_first(treemap *map) {
    if (map == NULL) return NULL;
    return treemap_minimum(map->root);
}

const treemap_node *treemap_first_const(const treemap *map) {
    if (map == NULL) return NULL;
    return treemap_minimum(map->root);
}

treemap_node *treemap_last(treemap *map) {
    if (map == NULL) return NULL;
    return treemap_maximum(map->root);
}

const treemap_node *treemap_last_const(const treemap *map) {
    if (map == NULL) return NULL;
    return treemap_maximum(map->root);
}

treemap_node *treemap_lower_bound(treemap *map, const void *key) {
    return treemap_find_bound_node(map, key, 0);
}

const treemap_node *treemap_lower_bound_const(const treemap *map, const void *key) {
    return treemap_find_bound_node(map, key, 0);
}

treemap_node *treemap_upper_bound(treemap *map, const void *key) {
    return treemap_find_bound_node(map, key, 1);
}

const treemap_node *treemap_upper_bound_const(const treemap *map, const void *key) {
    return treemap_find_bound_node(map, key, 1);
}

treemap_node *treemap_next(treemap_node *node) {
    if (node == NULL) return NULL;
    if (node->right != NULL) {
        return treemap_minimum(node->right);
    }

    while (node->parent != NULL && node == node->parent->right) {
        node = node->parent;
    }
    return node->parent;
}

const treemap_node *treemap_next_const(const treemap_node *node) {
    return treemap_next((treemap_node *)node);
}

treemap_node *treemap_prev(treemap_node *node) {
    if (node == NULL) return NULL;
    if (node->left != NULL) {
        return treemap_maximum(node->left);
    }

    while (node->parent != NULL && node == node->parent->left) {
        node = node->parent;
    }
    return node->parent;
}

const treemap_node *treemap_prev_const(const treemap_node *node) {
    return treemap_prev((treemap_node *)node);
}

void *treemap_node_key(treemap *map, treemap_node *node) {
    if (map == NULL || node == NULL) return NULL;
    return treemap_key_slot(map, node);
}

const void *treemap_node_key_const(const treemap *map, const treemap_node *node) {
    if (map == NULL || node == NULL) return NULL;
    return treemap_key_slot_const(map, node);
}

void *treemap_node_value(treemap *map, treemap_node *node) {
    if (map == NULL || node == NULL) return NULL;
    return treemap_value_slot(map, node);
}

const void *treemap_node_value_const(const treemap *map, const treemap_node *node) {
    if (map == NULL || node == NULL) return NULL;
    return treemap_value_slot_const(map, node);
}
