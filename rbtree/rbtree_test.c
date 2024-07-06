#include "rbtree.h"
#include "../testhelp.h"

#include <stdio.h>
#include <string.h>

char *strdup(const char *s) {
    char *t = malloc(strlen(s) + 1);
    if (t == NULL) return NULL;
    memcpy(t, s, strlen(s) + 1);
    return t;
}

int cmp(const int a, const int b) {
    return (a - b);
}

int cmpstr(const char *a, const char *b) {
    return strcmp(a, b);
}

int level = -1;
void rbt_print(rbt_node_int *n) {
    if (n == NULL) return;
    level++;
    rbt_print(n->right);
    level--;

    level++;
    for (int i = 0; i < level; i++) printf("\t");
    printf("%d%c\n", n->key, n->color == RBT_RED ? 'r': 'B');
    rbt_print(n->left);
    level--;
}

void rbtree_test_0(void) {
    rbtree_int *t = rbtree_new(t, cmp, NULL, NULL);
    rbt_insert(t, 3, 3);
    rbt_print(t->root);
    rbt_insert(t, 1, 1);
    rbt_print(t->root);
    rbt_insert(t, 2, 2);
    rbt_print(t->root);
    rbt_insert(t, 5, 5);
    rbt_print(t->root);
    rbt_insert(t, 4, 4);
    rbt_print(t->root);
    rbt_insert(t, 8, 8);
    rbt_print(t->root);
    rbt_insert(t, 0, 0);
    rbt_print(t->root);
    rbt_insert(t, 9, 9);
    rbt_print(t->root);
    rbt_insert(t, 7, 7);
    rbt_print(t->root);
    rbt_insert(t, 6, 6);
    rbt_print(t->root);

    rbt_find(t, 10);
    test_cond("rbtree", !rbt_found(t));
    rbt_find(t, 2);
    test_cond("rbtree", rbt_found(t));
    test_cond("rbtree", rbt_cur_value(t) == 2);

    rbt_update_begin(t);
    rbt_node_int *n = rbt_begin(t);
    while (n != NULL) {
        printf("%d ", n->value);
        rbt_increment(n);
    }
    printf("\n");

    rbt_update_end(t);
    n = rbt_end(t);
    while (n != NULL) {
        printf("%d ", n->value);
        rbt_decrement(n);
    }
    printf("\n");

    rbt_delete(t, 4);
    rbt_print(t->root);
    rbt_update_begin(t);
    n = rbt_begin(t);
    while (n != NULL) {
        printf("%d ", n->value);
        rbt_increment(n);
    }
    printf("\n");

    rbtree_free(t);
}

void rbtree_test_1(void) {
    rbtree_str *t = rbtree_new(t, cmpstr, free, free);
    rbt_insert(t, strdup("ab"), strdup("ab"));
    rbt_insert(t, strdup("cd"), strdup("cd"));
    rbt_insert(t, strdup("ef"), strdup("ef"));
    rbt_insert(t, strdup("gh"), strdup("gh"));
    rbt_find(t, "aa");
    test_cond("rbtree", !rbt_found(t));
    rbt_find(t, "cd");
    test_cond("rbtree", rbt_found(t));
    rbt_delete(t, "cd");
    rbt_find(t, "cd");
    test_cond("rbtree", !rbt_found(t));
    rbtree_free(t);
}

int main(void) {
    rbtree_test_0();
    rbtree_test_1();
    test_report();
}
