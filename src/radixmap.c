#include "radixmap.h"

#include <stdlib.h>
#include <string.h>

static int radixmap_copy_value(const radixmap *tree, void *dst, const void *src) {
    if (tree->value_copy != NULL) {
        return tree->value_copy(dst, src);
    }

    memcpy(dst, src, tree->value_size);
    return 0;
}

static void radixmap_destroy_value(const radixmap *tree, void *value) {
    if (tree->value_destroy != NULL) {
        tree->value_destroy(value);
    }
}

static radixmap_node *radixmap_node_new(const radixmap *tree, const char *segment, size_t segment_len) {
    radixmap_node *node = malloc(sizeof(*node) + tree->value_size);
    if (node == NULL) return NULL;

    node->segment = malloc(segment_len + 1);
    if (node->segment == NULL) {
        free(node);
        return NULL;
    }

    memcpy(node->segment, segment, segment_len);
    node->segment[segment_len] = '\0';
    node->next_sibling = NULL;
    node->child = NULL;
    node->segment_len = segment_len;
    node->has_value = 0;
    return node;
}

static void radixmap_node_free_chain(const radixmap *tree, radixmap_node *node) {
    if (node == NULL) return;

    radixmap_node_free_chain(tree, node->child);
    radixmap_node_free_chain(tree, node->next_sibling);
    if (node->has_value) {
        radixmap_destroy_value(tree, node->value);
    }
    free(node->segment);
    free(node);
}

static size_t radixmap_common_prefix_len(const char *segment, size_t segment_len, const char *key) {
    size_t common = 0;
    while (common < segment_len && key[common] != '\0' && segment[common] == key[common]) {
        common++;
    }
    return common;
}

static radixmap_node **radixmap_find_child_link(radixmap_node *node, char ch) {
    radixmap_node **link = &node->child;

    while (*link != NULL) {
        if ((*link)->segment[0] == ch) return link;
        link = &(*link)->next_sibling;
    }

    return link;
}

static int radixmap_store_value(const radixmap *tree, radixmap_node *node, const void *value) {
    if (!node->has_value) {
        if (radixmap_copy_value(tree, node->value, value) != 0) return -1;
        node->has_value = 1;
        return 0;
    }

    if (tree->value_copy == NULL) {
        radixmap_destroy_value(tree, node->value);
        memcpy(node->value, value, tree->value_size);
        return 0;
    }

    unsigned char *copy = malloc(tree->value_size);
    if (copy == NULL) return -1;
    if (radixmap_copy_value(tree, copy, value) != 0) {
        free(copy);
        return -1;
    }

    radixmap_destroy_value(tree, node->value);
    memcpy(node->value, copy, tree->value_size);
    free(copy);
    return 0;
}

static int radixmap_split_insert(radixmap *tree, radixmap_node **link,
                                 radixmap_node *node, const char *key,
                                 size_t common, const void *value) {
    size_t suffix_len = node->segment_len - common;
    char *suffix = malloc(suffix_len + 1);
    if (suffix == NULL) return -1;

    memcpy(suffix, node->segment + common, suffix_len);
    suffix[suffix_len] = '\0';

    radixmap_node *prefix = radixmap_node_new(tree, key, common);
    if (prefix == NULL) {
        free(suffix);
        return -1;
    }

    radixmap_node *leaf = NULL;
    if (key[common] == '\0') {
        if (radixmap_store_value(tree, prefix, value) != 0) {
            radixmap_node_free_chain(tree, prefix);
            free(suffix);
            return -1;
        }
    } else {
        leaf = radixmap_node_new(tree, key + common, strlen(key + common));
        if (leaf == NULL) {
            radixmap_node_free_chain(tree, prefix);
            free(suffix);
            return -1;
        }
        if (radixmap_store_value(tree, leaf, value) != 0) {
            radixmap_node_free_chain(tree, leaf);
            radixmap_node_free_chain(tree, prefix);
            free(suffix);
            return -1;
        }
    }

    prefix->next_sibling = node->next_sibling;
    prefix->child = node;

    free(node->segment);
    node->segment = suffix;
    node->segment_len = suffix_len;
    node->next_sibling = NULL;

    if (leaf != NULL) {
        leaf->next_sibling = prefix->child;
        prefix->child = leaf;
    }

    *link = prefix;
    tree->size++;
    return 0;
}

static const radixmap_node *radixmap_match_exact_const(const radixmap *tree, const char *key) {
    const radixmap_node *node = tree->root;

    while (*key != '\0') {
        const radixmap_node *child = node->child;
        while (child != NULL && child->segment[0] != *key) {
            child = child->next_sibling;
        }
        if (child == NULL) return NULL;

        size_t common = radixmap_common_prefix_len(child->segment, child->segment_len, key);
        if (common != child->segment_len) return NULL;
        key += common;
        node = child;
    }

    return node->has_value ? node : NULL;
}

static int radixmap_node_clone_value(const radixmap *tree, radixmap_node *dst, const radixmap_node *src) {
    if (!src->has_value) return 0;
    if (radixmap_copy_value(tree, dst->value, src->value) != 0) return -1;
    dst->has_value = 1;
    return 0;
}

static radixmap_node *radixmap_node_clone_chain(const radixmap *tree, const radixmap_node *src) {
    if (src == NULL) return NULL;

    radixmap_node *copy = radixmap_node_new(tree, src->segment, src->segment_len);
    if (copy == NULL) return NULL;

    if (radixmap_node_clone_value(tree, copy, src) != 0) {
        radixmap_node_free_chain(tree, copy);
        return NULL;
    }

    copy->child = radixmap_node_clone_chain(tree, src->child);
    if (src->child != NULL && copy->child == NULL) {
        radixmap_node_free_chain(tree, copy);
        return NULL;
    }

    copy->next_sibling = radixmap_node_clone_chain(tree, src->next_sibling);
    if (src->next_sibling != NULL && copy->next_sibling == NULL) {
        radixmap_node_free_chain(tree, copy);
        return NULL;
    }

    return copy;
}

static int radixmap_node_has_single_child(const radixmap_node *node) {
    return node->child != NULL && node->child->next_sibling == NULL;
}

static void radixmap_node_try_merge_child(const radixmap *tree, radixmap_node *node) {
    if (node->has_value || !radixmap_node_has_single_child(node)) return;

    radixmap_node *child = node->child;
    size_t segment_len = node->segment_len + child->segment_len;
    char *segment = malloc(segment_len + 1);
    if (segment == NULL) return;

    memcpy(segment, node->segment, node->segment_len);
    memcpy(segment + node->segment_len, child->segment, child->segment_len);
    segment[segment_len] = '\0';

    free(node->segment);
    node->segment = segment;
    node->segment_len = segment_len;
    node->child = child->child;
    node->has_value = child->has_value;
    if (child->has_value) {
        memcpy(node->value, child->value, tree->value_size);
        child->has_value = 0;
    }

    free(child->segment);
    free(child);
}

static int radixmap_delete_node(radixmap *tree, radixmap_node *node,
                             const char *key, int is_root,
                             int *remove_self) {
    if (*key == '\0') {
        if (!node->has_value) {
            *remove_self = 0;
            return 0;
        }

        radixmap_destroy_value(tree, node->value);
        node->has_value = 0;
        tree->size--;
    } else {
        radixmap_node **link = radixmap_find_child_link(node, *key);
        radixmap_node *child = *link;
        if (child == NULL) {
            *remove_self = 0;
            return 0;
        }

        size_t common = radixmap_common_prefix_len(child->segment, child->segment_len, key);
        if (common != child->segment_len) {
            *remove_self = 0;
            return 0;
        }

        int remove_child = 0;
        if (!radixmap_delete_node(tree, child, key + common, 0, &remove_child)) {
            *remove_self = 0;
            return 0;
        }

        if (remove_child) {
            *link = child->next_sibling;
            child->next_sibling = NULL;
            radixmap_node_free_chain(tree, child);
        }
    }

    if (!is_root && !node->has_value) {
        if (node->child == NULL) {
            *remove_self = 1;
            return 1;
        }

        radixmap_node_try_merge_child(tree, node);
    }

    *remove_self = 0;
    return 1;
}

int radixmap_init(radixmap *tree, size_t value_size,
                  radixmap_elem_destroy_fn value_destroy,
                  radixmap_elem_copy_fn value_copy) {
    if (tree == NULL || value_size == 0) return -1;

    tree->size = 0;
    tree->value_size = value_size;
    tree->value_destroy = value_destroy;
    tree->value_copy = value_copy;
    tree->root = radixmap_node_new(tree, "", 0);
    if (tree->root == NULL) return -1;
    return 0;
}

void radixmap_destroy(radixmap *tree) {
    if (tree == NULL || tree->root == NULL) return;

    radixmap_node_free_chain(tree, tree->root);
    tree->root = NULL;
    tree->size = 0;
    tree->value_size = 0;
    tree->value_destroy = NULL;
    tree->value_copy = NULL;
}

radixmap *radixmap_new(size_t value_size,
                       radixmap_elem_destroy_fn value_destroy,
                       radixmap_elem_copy_fn value_copy) {
    radixmap *tree = malloc(sizeof(*tree));
    if (tree == NULL) return NULL;
    if (radixmap_init(tree, value_size, value_destroy, value_copy) != 0) {
        free(tree);
        return NULL;
    }
    return tree;
}

void radixmap_free(radixmap *tree) {
    if (tree == NULL) return;
    radixmap_destroy(tree);
    free(tree);
}

int radixmap_copy(radixmap *dst, const radixmap *src) {
    if (dst == NULL || src == NULL || src->root == NULL) return -1;
    if (dst == src) return 0;

    radixmap temp;
    if (radixmap_init(&temp, src->value_size, src->value_destroy, src->value_copy) != 0) {
        return -1;
    }

    temp.size = src->size;
    temp.root->has_value = 0;
    if (src->root->has_value) {
        if (radixmap_store_value(&temp, temp.root, src->root->value) != 0) {
            radixmap_destroy(&temp);
            return -1;
        }
    }

    temp.root->child = radixmap_node_clone_chain(&temp, src->root->child);
    if (src->root->child != NULL && temp.root->child == NULL) {
        radixmap_destroy(&temp);
        return -1;
    }

    radixmap_destroy(dst);
    *dst = temp;
    return 0;
}

radixmap *radixmap_clone(const radixmap *src) {
    if (src == NULL) return NULL;

    radixmap *copy = radixmap_new(src->value_size, src->value_destroy, src->value_copy);
    if (copy == NULL) return NULL;
    if (radixmap_copy(copy, src) != 0) {
        radixmap_free(copy);
        return NULL;
    }
    return copy;
}

int radixmap_set(radixmap *tree, const char *key, const void *value) {
    if (tree == NULL || tree->root == NULL || key == NULL || value == NULL) return -1;

    radixmap_node *node = tree->root;
    while (1) {
        if (*key == '\0') {
            int had_value = node->has_value;
            if (radixmap_store_value(tree, node, value) != 0) return -1;
            if (!had_value) tree->size++;
            return 0;
        }

        radixmap_node **link = radixmap_find_child_link(node, *key);
        radixmap_node *child = *link;
        if (child == NULL) {
            radixmap_node *leaf = radixmap_node_new(tree, key, strlen(key));
            if (leaf == NULL) return -1;
            if (radixmap_store_value(tree, leaf, value) != 0) {
                radixmap_node_free_chain(tree, leaf);
                return -1;
            }
            *link = leaf;
            tree->size++;
            return 0;
        }

        size_t common = radixmap_common_prefix_len(child->segment, child->segment_len, key);
        if (common == child->segment_len) {
            node = child;
            key += common;
            continue;
        }

        return radixmap_split_insert(tree, link, child, key, common, value);
    }
}

int radixmap_contains(const radixmap *tree, const char *key) {
    return radixmap_at_const(tree, key) != NULL;
}

int radixmap_delete(radixmap *tree, const char *key) {
    if (tree == NULL || tree->root == NULL || key == NULL) return -1;

    int remove_root = 0;
    return radixmap_delete_node(tree, tree->root, key, 1, &remove_root) ? 0 : -1;
}

void radixmap_clear(radixmap *tree) {
    if (tree == NULL || tree->root == NULL) return;

    if (tree->root->has_value) {
        radixmap_destroy_value(tree, tree->root->value);
        tree->root->has_value = 0;
    }

    radixmap_node_free_chain(tree, tree->root->child);
    tree->root->child = NULL;
    tree->size = 0;
}

void *radixmap_at(radixmap *tree, const char *key) {
    return (void *)radixmap_at_const(tree, key);
}

const void *radixmap_at_const(const radixmap *tree, const char *key) {
    if (tree == NULL || tree->root == NULL || key == NULL) return NULL;

    const radixmap_node *node = radixmap_match_exact_const(tree, key);
    return node != NULL ? node->value : NULL;
}

int radixmap_has_prefix(const radixmap *tree, const char *prefix) {
    if (tree == NULL || tree->root == NULL || prefix == NULL) return 0;
    if (*prefix == '\0') return tree->size != 0;

    const radixmap_node *node = tree->root;
    while (*prefix != '\0') {
        const radixmap_node *child = node->child;
        while (child != NULL && child->segment[0] != *prefix) {
            child = child->next_sibling;
        }
        if (child == NULL) return 0;

        size_t common = radixmap_common_prefix_len(child->segment, child->segment_len, prefix);
        if (prefix[common] == '\0') return common > 0;
        if (common != child->segment_len) return 0;
        prefix += common;
        node = child;
    }

    return 1;
}
