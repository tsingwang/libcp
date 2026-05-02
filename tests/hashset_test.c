#include "hashset.h"
#include "testhelp.h"

#include <stdlib.h>
#include <string.h>

static int copy_count = 0;
static int copy_fail_after = -1;

static int byte_copy_with_fail(void *dst, const void *src) {
    if (copy_fail_after >= 0 && copy_count >= copy_fail_after) {
        return -1;
    }

    copy_count++;
    memcpy(dst, src, sizeof(unsigned char));
    return 0;
}

static void hashset_test_int_runtime(void) {
    hashset *set = hashset_new(HASHMAP_KEY_SCALAR, sizeof(int));
    int seen[6] = {0};
    size_t iter_count = 0;
    test_cond("hashset init", set != NULL);
    test_cond("hashset add", hashset_add_value(set, int, 3) == 0);
    test_cond("hashset add", hashset_add_value(set, int, 1) == 0);
    test_cond("hashset add", hashset_add_value(set, int, 5) == 0);
    test_cond("hashset add duplicate", hashset_add_value(set, int, 3) == 0);
    test_cond("hashset size", hashset_size(set) == 3);
    test_cond("hashset contains", hashset_contains_value(set, int, 5) == 1);
    test_cond("hashset reserve", hashset_reserve(set, 32) == 0);
    test_cond("hashset reserve capacity", hashset_capacity(set) >= 32);
    for (size_t i = hashset_first(set); i != HASHSET_NPOS; i = hashset_next(set, i)) {
        seen[*(int *)hashset_entry_key(set, i)] = 1;
        iter_count++;
    }
    test_cond("hashset iterate count", iter_count == 3);
    test_cond("hashset iterate seen", seen[1] && seen[3] && seen[5]);
    test_cond("hashset delete", hashset_delete_value(set, int, 1) == 0);
    test_cond("hashset delete missing", hashset_contains_value(set, int, 1) == 0);
    test_cond("hashset delete absent", hashset_delete_value(set, int, 1) == -1);
    hashset_clear(set);
    test_cond("hashset clear", hashset_empty(set));

    hashset_free(set);
}

static void hashset_test_copy_clone(void) {
    hashset src;
    hashset dst;
    hashset *clone;
    char key_buf[] = "alpha";

    test_cond("hashset copy init src",
              hashset_init(&src, HASHMAP_KEY_CSTR, sizeof(char *)) == 0);
    test_cond("hashset copy init dst",
              hashset_init(&dst, HASHMAP_KEY_SCALAR, sizeof(int)) == 0);
    test_cond("hashset copy fill", hashset_add_value(&src, char *, key_buf) == 0);
    test_cond("hashset copy fill", hashset_add_value(&src, char *, "beta") == 0);
    key_buf[0] = 'A';

    test_cond("hashset copy null dst", hashset_copy(NULL, &src) == -1);
    test_cond("hashset copy null src", hashset_copy(&dst, NULL) == -1);
    test_cond("hashset copy success", hashset_copy(&dst, &src) == 0);
    test_cond("hashset copy size", hashset_size(&dst) == 2);
    test_cond("hashset copy contents",
              hashset_contains_value(&dst, char *, "alpha") == 1 &&
              hashset_contains_value(&dst, char *, "beta") == 1);

    clone = hashset_clone(&src);
    test_cond("hashset clone alloc", clone != NULL);
    test_cond("hashset clone contents", hashset_contains_value(clone, char *, "alpha") == 1);
    test_cond("hashset clone independent", hashset_delete_value(clone, char *, "alpha") == 0);
    test_cond("hashset clone independent src", hashset_contains_value(&src, char *, "alpha") == 1);
    test_cond("hashset clone independent dst", hashset_contains_value(clone, char *, "alpha") == 0);

    hashset_free(clone);
    hashset_destroy(&dst);
    hashset_destroy(&src);
}

static void hashset_test_owned_runtime(void) {
    hashset *set = hashset_new(HASHMAP_KEY_CSTR, sizeof(char *));
    char key_buf[] = "alpha";

    test_cond("hashset ptr init", set != NULL);
    test_cond("hashset ptr add", hashset_add_value(set, char *, key_buf) == 0);
    test_cond("hashset ptr add", hashset_add_value(set, char *, "beta") == 0);
    key_buf[0] = 'A';
    test_cond("hashset ptr contains", hashset_contains_value(set, char *, "beta") == 1);
    test_cond("hashset ptr copied key", hashset_contains_value(set, char *, "alpha") == 1);
    test_cond("hashset ptr duplicate", hashset_add_value(set, char *, "alpha") == 0);
    test_cond("hashset ptr duplicate size", hashset_size(set) == 2);
    test_cond("hashset ptr delete", hashset_delete_value(set, char *, "alpha") == 0);
    hashset_clear(set);
    test_cond("hashset ptr clear", hashset_empty(set));

    hashset_free(set);
}

static void hashset_test_runtime_failures(void) {
    hashset set;
    test_cond("hashset init invalid", hashset_init(NULL, HASHMAP_KEY_SCALAR, sizeof(int)) == -1);
    test_cond("hashset init invalid kind",
              hashset_init(&set, (hashmap_key_kind)99, sizeof(int)) == -1);
    test_cond("hashset init invalid size", hashset_init(&set, HASHMAP_KEY_SCALAR, 0) == -1);
    test_cond("hashset init invalid cstr size",
              hashset_init(&set, HASHMAP_KEY_CSTR, sizeof(int)) == -1);
    test_cond("hashset init stack", hashset_init(&set, HASHMAP_KEY_SCALAR, sizeof(int)) == 0);
    test_cond("hashset reserve stack", hashset_reserve(&set, 8) == 0);
    test_cond("hashset add null", hashset_add(&set, NULL) == -1);
    test_cond("hashset contains empty", hashset_contains_value(&set, int, 1) == 0);
    hashset_destroy(&set);

    hashset *fail_set = hashset_new(HASHMAP_KEY_SCALAR, sizeof(int));
    fail_set->map.value_copy = byte_copy_with_fail;
    copy_count = 0;
    copy_fail_after = -1;
    test_cond("hashset init fail copy", fail_set != NULL);
    test_cond("hashset copy success", hashset_add_value(fail_set, int, 11) == 0);
    copy_fail_after = 1;
    test_cond("hashset copy failure add", hashset_add_value(fail_set, int, 22) == -1);
    test_cond("hashset copy failure size", hashset_size(fail_set) == 1);
    test_cond("hashset copy preserve", hashset_contains_value(fail_set, int, 11) == 1);
    copy_fail_after = -1;
    hashset_free(fail_set);

    hashset *fail_cstr = hashset_new(HASHMAP_KEY_CSTR, sizeof(char *));
    test_cond("hashset cstr init", fail_cstr != NULL);
    test_cond("hashset cstr null key", hashset_add(fail_cstr, &(char *){NULL}) == -1);
    hashset_free(fail_cstr);
}

int main(void) {
    hashset_test_int_runtime();
    hashset_test_owned_runtime();
    hashset_test_copy_clone();
    hashset_test_runtime_failures();
    test_report();
}
