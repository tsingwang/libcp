#ifndef __RBTREE_H
#define __RBTREE_H

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

/* free expected ‘void *’ when key or value is scalar */
#pragma GCC diagnostic ignored "-Wint-conversion"

enum rbt_color { RBT_RED, RBT_BLACK };

#define rbtree_def(name, K, V)                  \
    typedef struct rbt_node_##name {            \
        K key;                                  \
        V value;                                \
        enum rbt_color color:1;                 \
        struct rbt_node_##name *left;           \
        struct rbt_node_##name *right;          \
        struct rbt_node_##name *parent;         \
    } rbt_node_##name;                          \
    typedef struct {                            \
        size_t size;                            \
        rbt_node_##name *root;                  \
        int (*key_cmp_func)(const K, const K);  \
        void (*key_free_func)(void*);           \
        void (*value_free_func)(void*);         \
        rbt_node_##name *cursor; /* current find */  \
        rbt_node_##name *begin;  /* leftmost node */ \
        rbt_node_##name *end;    /* rightmost node */\
        rbt_node_##name *node;   /* temp node pointer */\
        rbt_node_##name *parent; /* temp node pointer */\
        rbt_node_##name *uncle;  /* temp node pointer */\
        rbt_node_##name *child;  /* temp node pointer */\
        rbt_node_##name *sibling;/* temp node pointer */\
        rbt_node_##name *temp;   /* temp node pointer */\
    } rbtree_##name

#define rbt_found(t) ((t)->cursor != NULL)
#define rbt_cur_key(t) ((t)->cursor->key)
#define rbt_cur_value(t) ((t)->cursor->value)
/* for better performance: The update operation is handed over to the user,
 * rather than automatically updating after inserting and deleting. */
#define rbt_begin(t) ((t)->begin)
#define rbt_end(t) ((t)->end)

#define rbt_empty(t) ((t)->size == 0)
#define rbt_size(t) ((t)->size)

#define rbtree_new(t, cmp, fk, fv)  \
    calloc(1, sizeof(*(t)));        \
    (t)->key_cmp_func = cmp;        \
    (t)->key_free_func = fk;        \
    (t)->value_free_func = fv;

#define rbtree_free(t)                                  \
    do {                                                \
        if ((t) == NULL) break;                         \
        rbt_update_begin(t);                            \
        (t)->node = rbt_begin(t);                       \
        while ((t)->node != NULL) {                     \
            (t)->temp = (t)->node;                      \
            rbt_increment((t)->node);                   \
            if ((t)->key_free_func) {                   \
                (t)->key_free_func((t)->temp->key);     \
            }                                           \
            if ((t)->value_free_func) {                 \
                (t)->value_free_func((t)->temp->value); \
            }                                           \
            free((t)->temp);                            \
        }                                               \
        free(t);                                        \
    } while (0)

#define rbt_find(t, _key)                                       \
    do {                                                        \
        if ((t) == NULL || (t)->root == NULL) break;            \
        (t)->cursor = (t)->root;                                \
        while ((t)->cursor != NULL) {                           \
            int cmp = (t)->key_cmp_func((t)->cursor->key, _key);\
            if (cmp > 0) {                                      \
                (t)->cursor = (t)->cursor->left;                \
            } else if (cmp < 0) {                               \
                (t)->cursor = (t)->cursor->right;               \
            } else {                                            \
                break;                                          \
            }                                                   \
        }                                                       \
    } while (0)

/* `node` is at the top
 *      B              D
 *     / \            / \
 *    A   D    =>    B   E
 *       / \        / \
 *      C   E      A   C
 */
#define _rbt_left_rotate(t, node)                   \
    do {                                            \
        (t)->temp = (node)->right;                  \
        (node)->right = (t)->temp->left;            \
        if ((t)->temp->left != NULL) {              \
            (t)->temp->left->parent = node;         \
        }                                           \
        (t)->temp->parent = (node)->parent;         \
        if ((node)->parent == NULL) {               \
            (t)->root = (t)->temp;                  \
        } else if (node == (node)->parent->left) {  \
            (node)->parent->left = (t)->temp;       \
        } else {                                    \
            (node)->parent->right = (t)->temp;      \
        }                                           \
        (t)->temp->left = node;                     \
        (node)->parent = (t)->temp;                 \
    } while (0)

/* `node` is at the top
 *        D          B
 *       / \        / \
 *      B   E  =>  A   D
 *     / \            / \
 *    A   C          C   E
 */
#define _rbt_right_rotate(t, node)                  \
    do {                                            \
        (t)->temp = (node)->left;                   \
        (node)->left = (t)->temp->right;            \
        if ((t)->temp->right != NULL) {             \
            (t)->temp->right->parent = node;        \
        }                                           \
        (t)->temp->parent = (node)->parent;         \
        if ((node)->parent == NULL) {               \
            (t)->root = (t)->temp;                  \
        } else if (node == (node)->parent->right) { \
            (node)->parent->right = (t)->temp;      \
        } else {                                    \
            (node)->parent->left = (t)->temp;       \
        }                                           \
        (t)->temp->right = node;                    \
        (node)->parent = (t)->temp;                 \
    } while (0)

#define _rbt_insert_fixup(t, node)                                      \
    do {                                                                \
        /* `node` is always a red node.  Initially, it is the newly inserted node.
         * Each iteration of this loop moves it higher up in the tree.
         */                                                             \
        while ((node) != (t)->root && (node)->parent->color == RBT_RED) { \
            if ((node)->parent == (node)->parent->parent->left) {       \
                (t)->uncle = (node)->parent->parent->right;             \
                if ((t)->uncle && (t)->uncle->color == RBT_RED) {       \
                    (node)->parent->color = RBT_BLACK;                  \
                    (t)->uncle->color = RBT_BLACK;                      \
                    (node)->parent->parent->color = RBT_RED;            \
                    node = (node)->parent->parent;                      \
                } else {                                                \
                    if (node == (node)->parent->right) {                \
                        node = (node)->parent;                          \
                        _rbt_left_rotate(t, node);                      \
                    }                                                   \
                    (node)->parent->color = RBT_BLACK;                  \
                    (node)->parent->parent->color = RBT_RED;            \
                    node = (node)->parent->parent;                      \
                    _rbt_right_rotate(t, node);                         \
                }                                                       \
            } else {                                                    \
                (t)->uncle = (node)->parent->parent->left;              \
                if ((t)->uncle && (t)->uncle->color == RBT_RED) {       \
                    (node)->parent->color = RBT_BLACK;                  \
                    (t)->uncle->color = RBT_BLACK;                      \
                    (node)->parent->parent->color = RBT_RED;            \
                    node = (node)->parent->parent;                      \
                } else {                                                \
                    if (node == (node)->parent->left) {                 \
                        node = (node)->parent;                          \
                        _rbt_right_rotate(t, node);                     \
                    }                                                   \
                    (node)->parent->color = RBT_BLACK;                  \
                    (node)->parent->parent->color = RBT_RED;            \
                    node = (node)->parent->parent;                      \
                    _rbt_left_rotate(t, node);                          \
                }                                                       \
            }                                                           \
        }                                                               \
        (t)->root->color = RBT_BLACK;                                   \
    } while (0)

#define rbt_insert(t, _key, _value)                             \
    do {                                                        \
        assert((t) != NULL);                                    \
        (t)->node = (t)->root;                                  \
        (t)->parent = NULL;                                     \
        int cmp = 0;                                            \
        while ((t)->node != NULL) {                             \
            (t)->parent = (t)->node;                            \
            cmp = (t)->key_cmp_func((t)->node->key, _key);      \
            if (cmp > 0) {                                      \
                (t)->node = (t)->node->left;                    \
            } else if (cmp < 0) {                               \
                (t)->node = (t)->node->right;                   \
            } else {                                            \
                if ((t)->value_free_func) {                     \
                    (t)->value_free_func((t)->node->value);     \
                }                                               \
                (t)->node->value = _value;                      \
                break;                                          \
            }                                                   \
        }                                                       \
        if ((t)->node != NULL) break; /* existed */             \
        (t)->node = malloc(sizeof(*(t)->node));                 \
        if ((t)->node == NULL) exit(1);                         \
        (t)->node->key = _key;                                  \
        (t)->node->value = _value;                              \
        (t)->node->color = RBT_RED; /* Initially, the node is red. */\
        (t)->node->left = (t)->node->right = NULL;              \
        (t)->node->parent = (t)->parent;                        \
        if ((t)->parent != NULL) {                              \
            if (cmp > 0) (t)->parent->left = (t)->node;         \
            else (t)->parent->right = (t)->node;                \
        } else {                                                \
            (t)->root = (t)->node;                              \
        }                                                       \
        (t)->size++;                                            \
        _rbt_insert_fixup(t, (t)->node);                        \
    } while (0)

#define _rbt_delete_fixup(t, node)                                      \
    do {                                                                \
        /*
         * `node` is always a black node.  Initially, it is the former child of the
         * deleted node.  Each iteration of this loop moves it higher up in the tree.
         */                                                             \
        while (node != (t)->root && (node)->color == RBT_BLACK) {       \
            if (node == (node)->parent->left) {                         \
                (t)->sibling = (node)->parent->right;                   \
                (t)->parent = (node)->parent; /* for macro expansion */ \
                if ((t)->sibling->color == RBT_RED) {                   \
                    (t)->sibling->color = RBT_BLACK;                    \
                    (node)->parent->color = RBT_RED;                    \
                    _rbt_left_rotate(t, (t)->parent);                   \
                    (t)->sibling = (node)->parent->right;               \
                }                                                       \
                if ((t)->sibling->left->color == RBT_BLACK &&           \
                        (t)->sibling->right->color == RBT_BLACK) {      \
                    (t)->sibling->color = RBT_RED;                      \
                    node = (node)->parent;                              \
                } else {                                                \
                    if ((t)->sibling->right->color == RBT_BLACK) {      \
                        (t)->sibling->left->color = RBT_BLACK;          \
                        (t)->sibling->color = RBT_RED;                  \
                        _rbt_right_rotate(t, (t)->sibling);             \
                        (t)->sibling = (node)->parent->right;           \
                    }                                                   \
                    (t)->sibling->color = (node)->parent->color;        \
                    (node)->parent->color = RBT_BLACK;                  \
                    (t)->sibling->right->color = RBT_BLACK;             \
                    _rbt_left_rotate(t, (t)->parent);                   \
                    node = (t)->root; /* Arrange for loop to terminate. */\
                }                                                       \
            } else {                                                    \
                (t)->sibling = (node)->parent->left;                    \
                (t)->parent = (node)->parent;                           \
                if ((t)->sibling->color == RBT_RED) {                   \
                    (t)->sibling->color = RBT_BLACK;                    \
                    (node)->parent->color = RBT_RED;                    \
                    _rbt_right_rotate(t, (t)->parent);                  \
                    (t)->sibling = (node)->parent->left;                \
                }                                                       \
                if ((t)->sibling->right->color == RBT_BLACK &&          \
                        (t)->sibling->left->color == RBT_BLACK) {       \
                    (t)->sibling->color = RBT_RED;                      \
                    node = (node)->parent;                              \
                } else {                                                \
                    if ((t)->sibling->left->color == RBT_BLACK) {       \
                        (t)->sibling->right->color = RBT_BLACK;         \
                        (t)->sibling->color = RBT_RED;                  \
                        _rbt_left_rotate(t, (t)->sibling);              \
                        (t)->sibling = (node)->parent->left;            \
                    }                                                   \
                    (t)->sibling->color = (node)->parent->color;        \
                    (node)->parent->color = RBT_BLACK;                  \
                    (t)->sibling->left->color = RBT_BLACK;              \
                    _rbt_right_rotate(t, (t)->parent);                  \
                    node = (t)->root; /* Arrange for loop to terminate. */\
                }                                                       \
            }                                                           \
        }                                                               \
        (node)->color = RBT_BLACK;                                      \
    } while (0)

#define rbt_delete(t, _key)                                     \
    do {                                                        \
        assert((t) != NULL && (t)->root != NULL);               \
        (t)->node = (t)->root;                                  \
        while ((t)->node != NULL) {                             \
            int cmp = (t)->key_cmp_func((t)->node->key, _key);  \
            if (cmp > 0) {                                      \
                (t)->node = (t)->node->left;                    \
            } else if (cmp < 0) {                               \
                (t)->node = (t)->node->right;                   \
            } else {                                            \
                break; /* found */                              \
            }                                                   \
        }                                                       \
        if ((t)->node == NULL) break; /* not existed */         \
        /* `temp` is the node that will actually be removed from the tree.
         * It will be `node` if `node` has fewer than two children,
         * or the tree successor of `node` otherwise. */        \
        if ((t)->node->left == NULL || (t)->node->right == NULL) { \
            (t)->temp = (t)->node;                              \
        } else { /* find tree successor */                      \
            (t)->temp = (t)->node->right;                       \
            while ((t)->temp->left != NULL)                     \
                (t)->temp = (t)->temp->left;                    \
        }                                                       \
        /* `child` is only one child of `temp` */               \
        if ((t)->temp->left != NULL)                            \
            (t)->child = (t)->temp->left;                       \
        else                                                    \
            (t)->child = (t)->temp->right;                      \
        /* Remove `temp` from the tree. */                      \
        if ((t)->child) (t)->child->parent = (t)->temp->parent; \
        if ((t)->temp->parent) {                                \
            if ((t)->temp == (t)->temp->parent->left)           \
                (t)->temp->parent->left = (t)->child;           \
            else                                                \
                (t)->temp->parent->right = (t)->child;          \
        } else {                                                \
            (t)->root = (t)->child;                             \
        }                                                       \
        /* Move successor `temp` data to `node` */              \
        if ((t)->key_free_func) {                               \
            (t)->key_free_func((t)->node->key);                 \
        }                                                       \
        if ((t)->value_free_func) {                             \
            (t)->value_free_func((t)->node->value);             \
        }                                                       \
        if ((t)->temp != (t)->node) {                           \
            (t)->node->key = (t)->temp->key;                    \
            (t)->node->value = (t)->temp->value;                \
        }                                                       \
        enum rbt_color temp_color = (t)->temp->color;           \
        free((t)->temp);                                        \
        /* Removing a black node might make some paths from root to leaf contain
         * fewer black nodes than others, or it might make two red nodes adjacent.*/\
        if (temp_color == RBT_BLACK)                            \
            _rbt_delete_fixup(t, (t)->child);                   \
        (t)->size--;                                            \
    } while (0)

#define rbt_update_begin(t)                     \
    do {                                        \
        if ((t) == NULL || (t)->root == NULL) { \
            (t)->begin = NULL;                  \
            break;                              \
        }                                       \
        (t)->begin = (t)->root;                 \
        while ((t)->begin->left != NULL) {      \
            (t)->begin = (t)->begin->left;      \
        }                                       \
    } while (0)

#define rbt_update_end(t)                       \
    do {                                        \
        if ((t) == NULL || (t)->root == NULL) { \
            (t)->end = NULL;                    \
            break;                              \
        }                                       \
        (t)->end = (t)->root;                   \
        while ((t)->end->right != NULL) {       \
            (t)->end = (t)->end->right;         \
        }                                       \
    } while (0)

#define rbt_increment(it)                       \
    do {                                        \
        if (it == NULL) break;                  \
        if ((it)->right != NULL) {              \
            it = (it)->right;                   \
            while ((it)->left != NULL) {        \
                it = (it)->left;                \
            }                                   \
        } else {                                \
            while ((it)->parent != NULL && it == (it)->parent->right) { \
                it = (it)->parent;              \
            }                                   \
            it = (it)->parent;                  \
        }                                       \
    } while (0)

#define rbt_decrement(it)                       \
    do {                                        \
        if (it == NULL) break;                  \
        if ((it)->left != NULL) {               \
            it = (it)->left;                    \
            while ((it)->right != NULL) {       \
                it = (it)->right;               \
            }                                   \
        } else {                                \
            while ((it)->parent != NULL && it == (it)->parent->left) { \
                it = (it)->parent;              \
            }                                   \
            it = (it)->parent;                  \
        }                                       \
    } while (0)

//        (name, K,     V)
rbtree_def(int,  int,   int);
rbtree_def(ints, int,   char*);
rbtree_def(str,  char*, char*);
rbtree_def(sint, char*, int);

#endif
