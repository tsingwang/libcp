#include "rbtree_typed.h"
#include "testhelp.h"

#include <stdlib.h>
#include <string.h>

static char *test_strdup(const char *s) {
    char *t = malloc(strlen(s) + 1);
    if (t == NULL) return NULL;
    memcpy(t, s, strlen(s) + 1);
    return t;
}

static int int_cmp(const int a, const int b) {
    return a - b;
}

static int str_cmp(const char *a, const char *b) {
    return strcmp(a, b);
}

static void rbtree_typed_test_int_map(void) {
    rbtree_int *tree = rbtree_new(tree, int_cmp, NULL, NULL);
    int ordered[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int reverse[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    size_t i = 0;
    rbt_node_int *node;

    test_cond("typed rbtree int init", tree != NULL);
    for (int v = 0; v < 10; v++) {
        rbt_insert(tree, v, v);
    }

    rbt_find(tree, 10);
    test_cond("typed rbtree int missing", !rbt_found(tree));
    rbt_find(tree, 2);
    test_cond("typed rbtree int found", rbt_found(tree));
    test_cond("typed rbtree int value", rbt_cur_value(tree) == 2);

    rbt_update_begin(tree);
    node = rbt_begin(tree);
    i = 0;
    while (node != NULL) {
        test_cond("typed rbtree int forward", node->value == ordered[i]);
        rbt_increment(node);
        i++;
    }
    test_cond("typed rbtree int forward count", i == 10);

    rbt_update_end(tree);
    node = rbt_end(tree);
    i = 0;
    while (node != NULL) {
        test_cond("typed rbtree int reverse", node->value == reverse[i]);
        rbt_decrement(node);
        i++;
    }
    test_cond("typed rbtree int reverse count", i == 10);

    rbt_insert(tree, 4, 44);
    rbt_find(tree, 4);
    test_cond("typed rbtree int update", rbt_found(tree));
    test_cond("typed rbtree int update value", rbt_cur_value(tree) == 44);

}

static void rbtree_typed_test_str_map(void) {
    rbtree_str *tree = rbtree_new(tree, str_cmp, free, free);

    test_cond("typed rbtree str init", tree != NULL);
    rbt_insert(tree, test_strdup("ab"), test_strdup("ab"));
    rbt_insert(tree, test_strdup("cd"), test_strdup("cd"));
    rbt_insert(tree, test_strdup("ef"), test_strdup("ef"));
    rbt_find(tree, "aa");
    test_cond("typed rbtree str missing", !rbt_found(tree));
    rbt_find(tree, "cd");
    test_cond("typed rbtree str found", rbt_found(tree));
    rbt_insert(tree, test_strdup("cd"), test_strdup("xy"));
    rbt_find(tree, "cd");
    test_cond("typed rbtree str update", rbt_found(tree));
    test_cond("typed rbtree str update value", strcmp(rbt_cur_value(tree), "xy") == 0);

}

int main(void) {
    rbtree_typed_test_int_map();
    rbtree_typed_test_str_map();
    test_report();
}
