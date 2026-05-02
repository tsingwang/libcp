#include "treemap.h"
#include "testhelp.h"

#include <stdlib.h>
#include <string.h>

static int destroy_count = 0;
static int copy_count = 0;
static int copy_fail_after = -1;

static int int_cmp(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

static int cstr_cmp(const void *a, const void *b) {
    return strcmp(*(char *const *)a, *(char *const *)b);
}

static int strdup_copy(void *dst, const void *src) {
    const char *value = *(char *const *)src;
    char *copy = malloc(strlen(value) + 1);
    if (copy == NULL) return -1;
    memcpy(copy, value, strlen(value) + 1);
    *(char **)dst = copy;
    return 0;
}

static void tracked_str_destroy(void *elem) {
    destroy_count++;
    free(*(char **)elem);
}

static int int_copy_with_fail(void *dst, const void *src) {
    if (copy_fail_after >= 0 && copy_count >= copy_fail_after) {
        return -1;
    }

    copy_count++;
    memcpy(dst, src, sizeof(int));
    return 0;
}

static void treemap_test_int_runtime(void) {
    treemap *map = treemap_new(TREEMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                               int_cmp, NULL, NULL);
    int forward[] = {1, 2, 3, 4, 5};
    int reverse[] = {5, 4, 3, 2, 1};
    int after_delete[] = {1, 2, 4, 5};
    size_t i = 0;
    treemap_node *node;

    test_cond("treemap init", map != NULL);
    test_cond("treemap set", treemap_set_value(map, int, 3, int, 30) == 0);
    test_cond("treemap set", treemap_set_value(map, int, 1, int, 10) == 0);
    test_cond("treemap set", treemap_set_value(map, int, 5, int, 50) == 0);
    test_cond("treemap set", treemap_set_value(map, int, 2, int, 20) == 0);
    test_cond("treemap set", treemap_set_value(map, int, 4, int, 40) == 0);
    test_cond("treemap size", treemap_size(map) == 5);
    test_cond("treemap get", *treemap_get_as(map, int, 2, int) == 20);
    test_cond("treemap contains", treemap_contains_value(map, int, 4) == 1);
    test_cond("treemap missing", treemap_get_as(map, int, 9, int) == NULL);
    test_cond("treemap update", treemap_set_value(map, int, 2, int, 99) == 0);
    test_cond("treemap update value", *treemap_get_as(map, int, 2, int) == 99);
    test_cond("treemap lower bound exact",
              *treemap_node_key_as(map, treemap_lower_bound(map, &(int){2}), int) == 2);
    test_cond("treemap lower bound gap",
              *treemap_node_key_as(map, treemap_lower_bound(map, &(int){0}), int) == 1);
    test_cond("treemap upper bound",
              *treemap_node_key_as(map, treemap_upper_bound(map, &(int){2}), int) == 3);
    test_cond("treemap upper bound missing high",
              treemap_upper_bound(map, &(int){5}) == NULL);

    for (node = treemap_first(map), i = 0; node != NULL;
         node = treemap_next(node), i++) {
        test_cond("treemap forward key",
                  *treemap_node_key_as(map, node, int) == forward[i]);
    }
    test_cond("treemap forward count", i == 5);

    for (node = treemap_last(map), i = 0; node != NULL;
         node = treemap_prev(node), i++) {
        test_cond("treemap reverse key",
                  *treemap_node_key_as(map, node, int) == reverse[i]);
    }
    test_cond("treemap reverse count", i == 5);

    test_cond("treemap delete", treemap_delete_value(map, int, 3) == 0);
    test_cond("treemap delete missing", treemap_contains_value(map, int, 3) == 0);
    for (node = treemap_first(map), i = 0; node != NULL;
         node = treemap_next(node), i++) {
        test_cond("treemap delete order",
                  *treemap_node_key_as(map, node, int) == after_delete[i]);
    }
    test_cond("treemap delete count", i == 4);

    treemap_clear(map);
    test_cond("treemap clear", treemap_empty(map));
    test_cond("treemap first empty", treemap_first(map) == NULL);
    test_cond("treemap last empty", treemap_last(map) == NULL);

    treemap_free(map);
}

static void treemap_test_copy_clone(void) {
    treemap src;
    treemap dst;
    treemap *clone;
    char key_buf[] = "alpha";

    destroy_count = 0;
    test_cond("treemap copy init src",
              treemap_init(&src, TREEMAP_KEY_CSTR, sizeof(char *), sizeof(char *),
                           cstr_cmp, tracked_str_destroy, strdup_copy) == 0);
    test_cond("treemap copy init dst",
              treemap_init(&dst, TREEMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                           int_cmp, NULL, NULL) == 0);
    test_cond("treemap copy fill",
              treemap_set_value(&src, char *, key_buf, char *, "one") == 0);
    test_cond("treemap copy fill",
              treemap_set_value(&src, char *, "beta", char *, "two") == 0);
    key_buf[0] = 'A';

    test_cond("treemap copy null dst", treemap_copy(NULL, &src) == -1);
    test_cond("treemap copy null src", treemap_copy(&dst, NULL) == -1);
    test_cond("treemap copy success", treemap_copy(&dst, &src) == 0);
    test_cond("treemap copy contents",
              strcmp(*treemap_get_as(&dst, char *, "alpha", char *), "one") == 0 &&
              strcmp(*treemap_get_as(&dst, char *, "beta", char *), "two") == 0);

    clone = treemap_clone(&src);
    test_cond("treemap clone alloc", clone != NULL);
    test_cond("treemap clone contents",
              strcmp(*treemap_get_as(clone, char *, "alpha", char *), "one") == 0);
    test_cond("treemap clone update",
              treemap_set_value(clone, char *, "beta", char *, "dos") == 0);
    test_cond("treemap clone independent",
              strcmp(*treemap_get_as(clone, char *, "beta", char *), "dos") == 0 &&
              strcmp(*treemap_get_as(&src, char *, "beta", char *), "two") == 0);

    treemap_free(clone);
    treemap_destroy(&dst);
    treemap_destroy(&src);
}

static void treemap_test_cstr_runtime(void) {
    treemap *map = treemap_new(TREEMAP_KEY_CSTR, sizeof(char *), sizeof(char *),
                               cstr_cmp, tracked_str_destroy, strdup_copy);
    char key_buf[] = "alpha";

    destroy_count = 0;
    test_cond("treemap cstr init", map != NULL);
    test_cond("treemap cstr set",
              treemap_set_value(map, char *, key_buf, char *, "one") == 0);
    test_cond("treemap cstr set",
              treemap_set_value(map, char *, "beta", char *, "two") == 0);
    test_cond("treemap cstr set",
              treemap_set_value(map, char *, "gamma", char *, "three") == 0);
    key_buf[0] = 'A';
    test_cond("treemap cstr copied key",
              strcmp(*treemap_get_as(map, char *, "alpha", char *), "one") == 0);
    test_cond("treemap cstr update",
              treemap_set_value(map, char *, "beta", char *, "dos") == 0);
    test_cond("treemap cstr update value",
              strcmp(*treemap_get_as(map, char *, "beta", char *), "dos") == 0);
    test_cond("treemap cstr update destroy", destroy_count == 1);
    test_cond("treemap cstr delete",
              treemap_delete_value(map, char *, "alpha") == 0);
    test_cond("treemap cstr delete destroy", destroy_count == 2);
    treemap_clear(map);
    test_cond("treemap cstr clear", treemap_empty(map));
    test_cond("treemap cstr clear destroy", destroy_count == 4);

    treemap_free(map);
}

static void treemap_test_runtime_failures(void) {
    treemap map;
    treemap *fail_map;
    treemap *cstr_map;

    test_cond("treemap init invalid",
              treemap_init(NULL, TREEMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                           int_cmp, NULL, NULL) == -1);
    test_cond("treemap init invalid kind",
              treemap_init(&map, (treemap_key_kind)99, sizeof(int), sizeof(int),
                           int_cmp, NULL, NULL) == -1);
    test_cond("treemap init invalid key",
              treemap_init(&map, TREEMAP_KEY_SCALAR, 0, sizeof(int),
                           int_cmp, NULL, NULL) == -1);
    test_cond("treemap init invalid value",
              treemap_init(&map, TREEMAP_KEY_SCALAR, sizeof(int), 0,
                           int_cmp, NULL, NULL) == -1);
    test_cond("treemap init invalid cmp",
              treemap_init(&map, TREEMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                           NULL, NULL, NULL) == -1);
    test_cond("treemap init invalid cstr size",
              treemap_init(&map, TREEMAP_KEY_CSTR, sizeof(int), sizeof(int),
                           cstr_cmp, NULL, NULL) == -1);
    test_cond("treemap new invalid",
              treemap_new((treemap_key_kind)99, sizeof(int), sizeof(int),
                          int_cmp, NULL, NULL) == NULL);
    test_cond("treemap init stack",
              treemap_init(&map, TREEMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                           int_cmp, NULL, NULL) == 0);
    test_cond("treemap set null key", treemap_set(&map, NULL, &(int){1}) == -1);
    test_cond("treemap set null value", treemap_set(&map, &(int){1}, NULL) == -1);
    test_cond("treemap contains empty", treemap_contains(&map, &(int){1}) == 0);
    test_cond("treemap delete empty", treemap_delete(&map, &(int){1}) == -1);
    treemap_destroy(&map);

    fail_map = treemap_new(TREEMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                           int_cmp, NULL, int_copy_with_fail);
    copy_count = 0;
    copy_fail_after = -1;
    test_cond("treemap init fail copy", fail_map != NULL);
    test_cond("treemap copy success",
              treemap_set_value(fail_map, int, 1, int, 11) == 0);
    copy_fail_after = 1;
    test_cond("treemap copy failure insert",
              treemap_set_value(fail_map, int, 2, int, 22) == -1);
    test_cond("treemap copy failure size", treemap_size(fail_map) == 1);
    test_cond("treemap copy preserve",
              *treemap_get_as(fail_map, int, 1, int) == 11);
    test_cond("treemap copy failure update",
              treemap_set_value(fail_map, int, 1, int, 33) == -1);
    test_cond("treemap update preserve",
              *treemap_get_as(fail_map, int, 1, int) == 11);
    copy_fail_after = -1;
    treemap_free(fail_map);

    cstr_map = treemap_new(TREEMAP_KEY_CSTR, sizeof(char *), sizeof(int),
                           cstr_cmp, NULL, NULL);
    test_cond("treemap cstr init fail", cstr_map != NULL);
    test_cond("treemap cstr null key",
              treemap_set(cstr_map, &(char *){NULL}, &(int){1}) == -1);
    treemap_free(cstr_map);
}

int main(void) {
    treemap_test_int_runtime();
    treemap_test_cstr_runtime();
    treemap_test_copy_clone();
    treemap_test_runtime_failures();
    test_report();
}
