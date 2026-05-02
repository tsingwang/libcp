#include "radix.h"
#include "testhelp.h"

static void radix_test_runtime(void) {
    radix *set = radix_new();
    char key_buf[] = "foobar";

    test_cond("radix init", set != NULL);
    test_cond("radix add empty", radix_add(set, "") == 0);
    test_cond("radix add", radix_add(set, key_buf) == 0);
    test_cond("radix add", radix_add(set, "foo") == 0);
    test_cond("radix add", radix_add(set, "food") == 0);
    test_cond("radix add duplicate", radix_add(set, "foo") == 0);
    key_buf[0] = 'F';

    test_cond("radix size", radix_size(set) == 4);
    test_cond("radix contains empty", radix_contains(set, "") == 1);
    test_cond("radix contains", radix_contains(set, "foobar") == 1);
    test_cond("radix contains split", radix_contains(set, "foo") == 1);
    test_cond("radix missing", radix_contains(set, "fo") == 0);
    test_cond("radix prefix foo", radix_has_prefix(set, "foo") == 1);
    test_cond("radix prefix fo", radix_has_prefix(set, "fo") == 1);
    test_cond("radix prefix foods", radix_has_prefix(set, "foods") == 0);
    test_cond("radix prefix empty", radix_has_prefix(set, "") == 1);

    test_cond("radix delete prefix", radix_delete(set, "foo") == 0);
    test_cond("radix delete prefix missing", radix_contains(set, "foo") == 0);
    test_cond("radix preserve sibling", radix_contains(set, "foobar") == 1);
    test_cond("radix preserve branch", radix_contains(set, "food") == 1);
    test_cond("radix delete branch", radix_delete(set, "food") == 0);
    test_cond("radix merged remainder", radix_contains(set, "foobar") == 1);
    test_cond("radix prefix after delete", radix_has_prefix(set, "foo") == 1);
    test_cond("radix delete leaf", radix_delete(set, "foobar") == 0);
    test_cond("radix prefix cleared", radix_has_prefix(set, "foo") == 0);
    test_cond("radix delete absent", radix_delete(set, "foobar") == -1);
    radix_clear(set);
    test_cond("radix clear", radix_empty(set));
    test_cond("radix prefix after clear", radix_has_prefix(set, "") == 0);

    radix_free(set);
}

static void radix_test_copy_clone(void) {
    radix src;
    radix dst;
    radix *clone;
    char key_buf[] = "alpha";

    test_cond("radix copy init src", radix_init(&src) == 0);
    test_cond("radix copy init dst", radix_init(&dst) == 0);
    test_cond("radix copy fill src", radix_add(&src, key_buf) == 0);
    test_cond("radix copy fill src", radix_add(&src, "alphabet") == 0);
    test_cond("radix copy fill src", radix_add(&src, "") == 0);
    key_buf[0] = 'A';

    test_cond("radix copy null dst", radix_copy(NULL, &src) == -1);
    test_cond("radix copy null src", radix_copy(&dst, NULL) == -1);
    test_cond("radix copy success", radix_copy(&dst, &src) == 0);
    test_cond("radix copy size", radix_size(&dst) == 3);
    test_cond("radix copy contents",
              radix_contains(&dst, "alpha") == 1 &&
              radix_contains(&dst, "alphabet") == 1 &&
              radix_contains(&dst, "") == 1);
    test_cond("radix copy prefix", radix_has_prefix(&dst, "alph") == 1);

    clone = radix_clone(&src);
    test_cond("radix clone alloc", clone != NULL);
    test_cond("radix clone contents", radix_contains(clone, "alpha") == 1);
    test_cond("radix clone update", radix_add(clone, "alphanumeric") == 0);
    test_cond("radix clone independent",
              radix_contains(clone, "alphanumeric") == 1 &&
              radix_contains(&src, "alphanumeric") == 0);
    test_cond("radix clone delete", radix_delete(clone, "") == 0);
    test_cond("radix source unchanged", radix_contains(&src, "") == 1);

    radix_free(clone);
    radix_destroy(&dst);
    radix_destroy(&src);
}

static void radix_test_runtime_failures(void) {
    radix set;

    test_cond("radix init invalid", radix_init(NULL) == -1);
    test_cond("radix init stack", radix_init(&set) == 0);
    test_cond("radix add null key", radix_add(&set, NULL) == -1);
    test_cond("radix contains empty", radix_contains(&set, "x") == 0);
    test_cond("radix contains null key", radix_contains(&set, NULL) == 0);
    test_cond("radix prefix null", radix_has_prefix(&set, NULL) == 0);
    test_cond("radix delete null key", radix_delete(&set, NULL) == -1);
    test_cond("radix delete empty missing", radix_delete(&set, "") == -1);
    radix_destroy(&set);

    radix *heap = radix_new();
    test_cond("radix new alloc", heap != NULL);
    radix_free(heap);
    test_cond("radix clone null", radix_clone(NULL) == NULL);
}

int main(void) {
    radix_test_runtime();
    radix_test_copy_clone();
    radix_test_runtime_failures();
    test_report();
}
