#include "radixmap.h"
#include "testhelp.h"

#include <stdlib.h>
#include <string.h>

static int destroy_count = 0;
static int copy_count = 0;
static int copy_fail_after = -1;

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

static void radixmap_test_int_runtime(void) {
    radixmap *tree = radixmap_new(sizeof(int), NULL, NULL);
    char key_buf[] = "foobar";

    test_cond("radixmap init", tree != NULL);
    test_cond("radixmap set empty", radixmap_set_value(tree, "", int, 1) == 0);
    test_cond("radixmap set", radixmap_set_value(tree, key_buf, int, 10) == 0);
    test_cond("radixmap set", radixmap_set_value(tree, "foo", int, 5) == 0);
    test_cond("radixmap set", radixmap_set_value(tree, "food", int, 20) == 0);
    test_cond("radixmap set", radixmap_set_value(tree, "bar", int, 30) == 0);
    key_buf[0] = 'F';

    test_cond("radixmap size", radixmap_size(tree) == 5);
    test_cond("radixmap get empty", *radixmap_get_as(tree, "", int) == 1);
    test_cond("radixmap get", *radixmap_get_as(tree, "foobar", int) == 10);
    test_cond("radixmap get split key", *radixmap_get_as(tree, "foo", int) == 5);
    test_cond("radixmap missing", radixmap_get_as(tree, "fo", int) == NULL);
    test_cond("radixmap contains", radixmap_contains_value(tree, "food") == 1);
    test_cond("radixmap prefix foo", radixmap_has_prefix(tree, "foo") == 1);
    test_cond("radixmap prefix fo", radixmap_has_prefix(tree, "fo") == 1);
    test_cond("radixmap prefix foods", radixmap_has_prefix(tree, "foods") == 0);
    test_cond("radixmap prefix empty", radixmap_has_prefix(tree, "") == 1);

    test_cond("radixmap update", radixmap_set_value(tree, "food", int, 99) == 0);
    test_cond("radixmap update value", *radixmap_get_as(tree, "food", int) == 99);
    test_cond("radixmap delete prefix node", radixmap_delete_value(tree, "foo") == 0);
    test_cond("radixmap delete prefix missing", radixmap_contains_value(tree, "foo") == 0);
    test_cond("radixmap preserve sibling", *radixmap_get_as(tree, "foobar", int) == 10);
    test_cond("radixmap preserve branch", *radixmap_get_as(tree, "food", int) == 99);
    test_cond("radixmap delete branch", radixmap_delete_value(tree, "food") == 0);
    test_cond("radixmap merged remainder", *radixmap_get_as(tree, "foobar", int) == 10);
    test_cond("radixmap prefix after delete", radixmap_has_prefix(tree, "foo") == 1);
    test_cond("radixmap delete leaf", radixmap_delete_value(tree, "foobar") == 0);
    test_cond("radixmap prefix cleared", radixmap_has_prefix(tree, "foo") == 0);
    test_cond("radixmap delete absent", radixmap_delete_value(tree, "foobar") == -1);
    radixmap_clear(tree);
    test_cond("radixmap clear", radixmap_empty(tree));
    test_cond("radixmap prefix after clear", radixmap_has_prefix(tree, "") == 0);

    radixmap_free(tree);
}

static void radixmap_test_copy_clone(void) {
    radixmap src;
    radixmap dst;
    radixmap *clone;
    char key_buf[] = "alpha";

    destroy_count = 0;
    test_cond("radixmap copy init src",
              radixmap_init(&src, sizeof(char *), tracked_str_destroy, strdup_copy) == 0);
    test_cond("radixmap copy init dst",
              radixmap_init(&dst, sizeof(int), NULL, NULL) == 0);
    test_cond("radixmap copy fill src",
              radixmap_set_value(&src, key_buf, char *, "one") == 0);
    test_cond("radixmap copy fill src",
              radixmap_set_value(&src, "alphabet", char *, "two") == 0);
    test_cond("radixmap copy fill src",
              radixmap_set_value(&src, "", char *, "zero") == 0);
    key_buf[0] = 'A';

    test_cond("radixmap copy null dst", radixmap_copy(NULL, &src) == -1);
    test_cond("radixmap copy null src", radixmap_copy(&dst, NULL) == -1);
    test_cond("radixmap copy success", radixmap_copy(&dst, &src) == 0);
    test_cond("radixmap copy size", radixmap_size(&dst) == 3);
    test_cond("radixmap copy contents",
              strcmp(*radixmap_get_as(&dst, "alpha", char *), "one") == 0 &&
              strcmp(*radixmap_get_as(&dst, "alphabet", char *), "two") == 0 &&
              strcmp(*radixmap_get_as(&dst, "", char *), "zero") == 0);
    test_cond("radixmap copy prefix", radixmap_has_prefix(&dst, "alph") == 1);

    clone = radixmap_clone(&src);
    test_cond("radixmap clone alloc", clone != NULL);
    test_cond("radixmap clone contents",
              strcmp(*radixmap_get_as(clone, "alpha", char *), "one") == 0);
    test_cond("radixmap clone update",
              radixmap_set_value(clone, "alphabet", char *, "dos") == 0);
    test_cond("radixmap clone independent",
              strcmp(*radixmap_get_as(clone, "alphabet", char *), "dos") == 0 &&
              strcmp(*radixmap_get_as(&src, "alphabet", char *), "two") == 0);
    test_cond("radixmap clone delete", radixmap_delete_value(clone, "") == 0);
    test_cond("radixmap source unchanged", radixmap_contains_value(&src, "") == 1);

    radixmap_free(clone);
    radixmap_destroy(&dst);
    radixmap_destroy(&src);
}

static void radixmap_test_runtime_failures(void) {
    radixmap tree;

    test_cond("radixmap init invalid", radixmap_init(NULL, sizeof(int), NULL, NULL) == -1);
    test_cond("radixmap init invalid size", radixmap_init(&tree, 0, NULL, NULL) == -1);
    test_cond("radixmap new invalid", radixmap_new(0, NULL, NULL) == NULL);
    test_cond("radixmap init stack", radixmap_init(&tree, sizeof(int), NULL, NULL) == 0);
    test_cond("radixmap set null key", radixmap_set(&tree, NULL, &(int){1}) == -1);
    test_cond("radixmap set null value", radixmap_set(&tree, "a", NULL) == -1);
    test_cond("radixmap contains null key", radixmap_contains(&tree, NULL) == 0);
    test_cond("radixmap at null key", radixmap_at(&tree, NULL) == NULL);
    test_cond("radixmap prefix null", radixmap_has_prefix(&tree, NULL) == 0);
    test_cond("radixmap delete null key", radixmap_delete(&tree, NULL) == -1);
    test_cond("radixmap delete empty missing", radixmap_delete(&tree, "") == -1);
    radixmap_destroy(&tree);

    radixmap *fail_tree = radixmap_new(sizeof(int), NULL, int_copy_with_fail);
    copy_count = 0;
    copy_fail_after = -1;
    test_cond("radixmap init fail copy", fail_tree != NULL);
    test_cond("radixmap copy success", radixmap_set_value(fail_tree, "a", int, 11) == 0);
    copy_fail_after = 1;
    test_cond("radixmap copy failure insert", radixmap_set_value(fail_tree, "ab", int, 22) == -1);
    test_cond("radixmap copy failure size", radixmap_size(fail_tree) == 1);
    test_cond("radixmap copy preserve", *radixmap_get_as(fail_tree, "a", int) == 11);
    copy_fail_after = 1;
    test_cond("radixmap copy failure update", radixmap_set_value(fail_tree, "a", int, 33) == -1);
    test_cond("radixmap update preserve", *radixmap_get_as(fail_tree, "a", int) == 11);
    copy_fail_after = -1;
    radixmap_free(fail_tree);
}

int main(void) {
    radixmap_test_int_runtime();
    radixmap_test_copy_clone();
    radixmap_test_runtime_failures();
    test_report();
}
