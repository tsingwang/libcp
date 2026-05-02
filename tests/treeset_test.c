#include "treeset.h"
#include "testhelp.h"

#include <string.h>

static int int_cmp(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

static int cstr_cmp(const void *a, const void *b) {
    return strcmp(*(char *const *)a, *(char *const *)b);
}

static void treeset_test_int_runtime(void) {
    treeset *set = treeset_new(TREEMAP_KEY_SCALAR, sizeof(int), int_cmp);
    int forward[] = {1, 3, 5};
    int reverse[] = {5, 3, 1};
    size_t i = 0;
    treemap_node *node;

    test_cond("treeset init", set != NULL);
    test_cond("treeset add", treeset_add_value(set, int, 3) == 0);
    test_cond("treeset add", treeset_add_value(set, int, 1) == 0);
    test_cond("treeset add", treeset_add_value(set, int, 5) == 0);
    test_cond("treeset add duplicate", treeset_add_value(set, int, 3) == 0);
    test_cond("treeset size", treeset_size(set) == 3);
    test_cond("treeset contains", treeset_contains_value(set, int, 5) == 1);
    test_cond("treeset lower bound exact",
              *treeset_node_key_as(set, treeset_lower_bound(set, &(int){3}), int) == 3);
    test_cond("treeset lower bound gap",
              *treeset_node_key_as(set, treeset_lower_bound(set, &(int){2}), int) == 3);
    test_cond("treeset upper bound",
              *treeset_node_key_as(set, treeset_upper_bound(set, &(int){3}), int) == 5);
    test_cond("treeset upper bound missing high",
              treeset_upper_bound(set, &(int){5}) == NULL);

    for (node = treeset_first(set), i = 0; node != NULL;
         node = treeset_next(node), i++) {
        test_cond("treeset forward key",
                  *treeset_node_key_as(set, node, int) == forward[i]);
    }
    test_cond("treeset forward count", i == 3);

    for (node = treeset_last(set), i = 0; node != NULL;
         node = treeset_prev(node), i++) {
        test_cond("treeset reverse key",
                  *treeset_node_key_as(set, node, int) == reverse[i]);
    }
    test_cond("treeset reverse count", i == 3);

    test_cond("treeset delete", treeset_delete_value(set, int, 1) == 0);
    test_cond("treeset delete missing", treeset_contains_value(set, int, 1) == 0);
    treeset_clear(set);
    test_cond("treeset clear", treeset_empty(set));

    treeset_free(set);
}

static void treeset_test_copy_clone(void) {
    treeset src;
    treeset dst;
    treeset *clone;
    char key_buf[] = "alpha";

    test_cond("treeset copy init src",
              treeset_init(&src, TREEMAP_KEY_CSTR, sizeof(char *), cstr_cmp) == 0);
    test_cond("treeset copy init dst",
              treeset_init(&dst, TREEMAP_KEY_SCALAR, sizeof(int), int_cmp) == 0);
    test_cond("treeset copy fill", treeset_add_value(&src, char *, key_buf) == 0);
    test_cond("treeset copy fill", treeset_add_value(&src, char *, "beta") == 0);
    key_buf[0] = 'A';

    test_cond("treeset copy null dst", treeset_copy(NULL, &src) == -1);
    test_cond("treeset copy null src", treeset_copy(&dst, NULL) == -1);
    test_cond("treeset copy success", treeset_copy(&dst, &src) == 0);
    test_cond("treeset copy contents",
              treeset_contains_value(&dst, char *, "alpha") == 1 &&
              treeset_contains_value(&dst, char *, "beta") == 1);

    clone = treeset_clone(&src);
    test_cond("treeset clone alloc", clone != NULL);
    test_cond("treeset clone contents", treeset_contains_value(clone, char *, "alpha") == 1);
    test_cond("treeset clone independent", treeset_delete_value(clone, char *, "alpha") == 0);
    test_cond("treeset clone independent src", treeset_contains_value(&src, char *, "alpha") == 1);
    test_cond("treeset clone independent dst", treeset_contains_value(clone, char *, "alpha") == 0);

    treeset_free(clone);
    treeset_destroy(&dst);
    treeset_destroy(&src);
}

static void treeset_test_cstr_runtime(void) {
    treeset *set = treeset_new(TREEMAP_KEY_CSTR, sizeof(char *), cstr_cmp);
    char key_buf[] = "alpha";

    test_cond("treeset cstr init", set != NULL);
    test_cond("treeset cstr add", treeset_add_value(set, char *, key_buf) == 0);
    test_cond("treeset cstr add", treeset_add_value(set, char *, "beta") == 0);
    key_buf[0] = 'A';
    test_cond("treeset cstr copied key",
              treeset_contains_value(set, char *, "alpha") == 1);
    test_cond("treeset cstr duplicate",
              treeset_add_value(set, char *, "alpha") == 0);
    test_cond("treeset cstr duplicate size", treeset_size(set) == 2);
    test_cond("treeset cstr delete",
              treeset_delete_value(set, char *, "alpha") == 0);
    treeset_clear(set);
    test_cond("treeset cstr clear", treeset_empty(set));

    treeset_free(set);
}

static void treeset_test_runtime_failures(void) {
    treeset set;
    test_cond("treeset init invalid",
              treeset_init(NULL, TREEMAP_KEY_SCALAR, sizeof(int), int_cmp) == -1);
    test_cond("treeset init invalid kind",
              treeset_init(&set, (treemap_key_kind)99, sizeof(int), int_cmp) == -1);
    test_cond("treeset init invalid size",
              treeset_init(&set, TREEMAP_KEY_SCALAR, 0, int_cmp) == -1);
    test_cond("treeset init invalid cmp",
              treeset_init(&set, TREEMAP_KEY_SCALAR, sizeof(int), NULL) == -1);
    test_cond("treeset init invalid cstr size",
              treeset_init(&set, TREEMAP_KEY_CSTR, sizeof(int), cstr_cmp) == -1);
    test_cond("treeset init stack",
              treeset_init(&set, TREEMAP_KEY_SCALAR, sizeof(int), int_cmp) == 0);
    test_cond("treeset add null", treeset_add(&set, NULL) == -1);
    test_cond("treeset contains empty", treeset_contains_value(&set, int, 1) == 0);
    treeset_destroy(&set);
}

int main(void) {
    treeset_test_int_runtime();
    treeset_test_cstr_runtime();
    treeset_test_copy_clone();
    treeset_test_runtime_failures();
    test_report();
}
