#include "hashmap.h"
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

static void hashmap_test_int_runtime(void) {
    hashmap *map = hashmap_new(HASHMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                               NULL, NULL);
    int seen[4] = {0};
    int sum = 0;
    size_t iter_count = 0;
    test_cond("hashmap init", map != NULL);
    test_cond("hashmap set", hashmap_set_value(map, int, 1, int, 11) == 0);
    test_cond("hashmap set", hashmap_set_value(map, int, 2, int, 22) == 0);
    test_cond("hashmap set", hashmap_set_value(map, int, 3, int, 33) == 0);
    test_cond("hashmap size", hashmap_size(map) == 3);
    test_cond("hashmap get", *hashmap_get_as(map, int, 2, int) == 22);
    test_cond("hashmap contains", hashmap_contains_value(map, int, 3) == 1);
    test_cond("hashmap missing", hashmap_get_as(map, int, 9, int) == NULL);
    test_cond("hashmap update", hashmap_set_value(map, int, 2, int, 99) == 0);
    test_cond("hashmap update value", *hashmap_get_as(map, int, 2, int) == 99);
    test_cond("hashmap reserve", hashmap_reserve(map, 32) == 0);
    test_cond("hashmap reserve capacity", hashmap_capacity(map) >= 32);
    test_cond("hashmap preserve after reserve", *hashmap_get_as(map, int, 1, int) == 11);
    test_cond("hashmap const access", *hashmap_get_const_as(map, int, 3, int) == 33);
    for (size_t i = hashmap_first(map); i != HASHMAP_NPOS; i = hashmap_next(map, i)) {
        int key = *(int *)hashmap_entry_key(map, i);
        int value = *(int *)hashmap_entry_value(map, i);
        seen[key] = 1;
        sum += value;
        iter_count++;
    }
    test_cond("hashmap iterate count", iter_count == 3);
    test_cond("hashmap iterate seen", seen[1] && seen[2] && seen[3]);
    test_cond("hashmap iterate sum", sum == 11 + 99 + 33);
    test_cond("hashmap delete", hashmap_delete_value(map, int, 2) == 0);
    test_cond("hashmap delete missing", hashmap_contains_value(map, int, 2) == 0);
    test_cond("hashmap delete absent", hashmap_delete_value(map, int, 2) == -1);
    hashmap_clear(map);
    test_cond("hashmap clear", hashmap_empty(map));

    hashmap_free(map);
}

static void hashmap_test_copy_clone(void) {
    hashmap src;
    hashmap dst;
    hashmap *clone;
    char key_buf[] = "alpha";
    int sum = 0;
    size_t count = 0;

    destroy_count = 0;
    test_cond("hashmap copy init src",
              hashmap_init(&src, HASHMAP_KEY_CSTR, sizeof(char *), sizeof(char *),
                           tracked_str_destroy, strdup_copy) == 0);
    test_cond("hashmap copy init dst",
              hashmap_init(&dst, HASHMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                           NULL, NULL) == 0);
    test_cond("hashmap copy fill src",
              hashmap_set_value(&src, char *, key_buf, char *, "one") == 0);
    test_cond("hashmap copy fill src",
              hashmap_set_value(&src, char *, "beta", char *, "two") == 0);
    key_buf[0] = 'A';

    test_cond("hashmap copy null dst", hashmap_copy(NULL, &src) == -1);
    test_cond("hashmap copy null src", hashmap_copy(&dst, NULL) == -1);
    test_cond("hashmap copy success", hashmap_copy(&dst, &src) == 0);
    test_cond("hashmap copy metadata", dst.key_kind == HASHMAP_KEY_CSTR && dst.key_size == sizeof(char *));
    test_cond("hashmap copy size", hashmap_size(&dst) == 2);
    test_cond("hashmap copy contents",
              strcmp(*hashmap_get_as(&dst, char *, "alpha", char *), "one") == 0 &&
              strcmp(*hashmap_get_as(&dst, char *, "beta", char *), "two") == 0);

    clone = hashmap_clone(&src);
    test_cond("hashmap clone alloc", clone != NULL);
    test_cond("hashmap clone contents",
              strcmp(*hashmap_get_as(clone, char *, "alpha", char *), "one") == 0);
    test_cond("hashmap clone update",
              hashmap_set_value(clone, char *, "beta", char *, "dos") == 0);
    test_cond("hashmap clone independent",
              strcmp(*hashmap_get_as(clone, char *, "beta", char *), "dos") == 0 &&
              strcmp(*hashmap_get_as(&src, char *, "beta", char *), "two") == 0);

    for (size_t i = hashmap_first(clone); i != HASHMAP_NPOS; i = hashmap_next(clone, i)) {
        sum += (int)strlen(*(char **)hashmap_entry_value(clone, i));
        count++;
    }
    test_cond("hashmap clone iterate count", count == 2);
    test_cond("hashmap clone iterate values", sum == 3 + 3);

    hashmap_free(clone);
    hashmap_destroy(&dst);
    hashmap_destroy(&src);
}

static void hashmap_test_growth_runtime(void) {
    hashmap *map = hashmap_new(HASHMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                               NULL, NULL);
    test_cond("hashmap growth init", map != NULL);
    for (int i = 0; i < 12; i++) {
        test_cond("hashmap growth set", hashmap_set_value(map, int, i, int, i * 10) == 0);
    }
    test_cond("hashmap growth size", hashmap_size(map) == 12);
    test_cond("hashmap growth get", *hashmap_get_as(map, int, 7, int) == 70);
    test_cond("hashmap growth delete", hashmap_delete_value(map, int, 3) == 0);
    test_cond("hashmap growth preserve", *hashmap_get_as(map, int, 11, int) == 110);

    hashmap_free(map);
}

static void hashmap_test_owned_runtime(void) {
    hashmap *map = hashmap_new(HASHMAP_KEY_CSTR, sizeof(char *), sizeof(char *),
                               tracked_str_destroy, strdup_copy);
    char key_buf[] = "alpha";

    destroy_count = 0;
    test_cond("hashmap ptr init", map != NULL);
    test_cond("hashmap ptr set", hashmap_set_value(map, char *, key_buf, char *, "one") == 0);
    test_cond("hashmap ptr set", hashmap_set_value(map, char *, "beta", char *, "two") == 0);
    test_cond("hashmap ptr set", hashmap_set_value(map, char *, "gamma", char *, "three") == 0);
    key_buf[0] = 'A';
    test_cond("hashmap ptr get", strcmp(*hashmap_get_as(map, char *, "alpha", char *), "one") == 0);
    test_cond("hashmap ptr update", hashmap_set_value(map, char *, "beta", char *, "dos") == 0);
    test_cond("hashmap ptr update value",
              strcmp(*hashmap_get_as(map, char *, "beta", char *), "dos") == 0);
    test_cond("hashmap ptr update destroy", destroy_count == 1);
    test_cond("hashmap ptr delete", hashmap_delete_value(map, char *, "alpha") == 0);
    test_cond("hashmap ptr delete destroy", destroy_count == 2);
    hashmap_clear(map);
    test_cond("hashmap ptr clear", hashmap_empty(map));
    test_cond("hashmap ptr clear destroy", destroy_count == 4);

    hashmap_free(map);
}

static void hashmap_test_runtime_failures(void) {
    hashmap map;
    test_cond("hashmap init invalid",
              hashmap_init(NULL, HASHMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                           NULL, NULL) == -1);
    test_cond("hashmap init invalid kind",
              hashmap_init(&map, (hashmap_key_kind)99, sizeof(int), sizeof(int),
                           NULL, NULL) == -1);
    test_cond("hashmap init invalid key",
              hashmap_init(&map, HASHMAP_KEY_SCALAR, 0, sizeof(int),
                           NULL, NULL) == -1);
    test_cond("hashmap init invalid value",
              hashmap_init(&map, HASHMAP_KEY_SCALAR, sizeof(int), 0,
                           NULL, NULL) == -1);
    test_cond("hashmap init invalid cstr size",
              hashmap_init(&map, HASHMAP_KEY_CSTR, sizeof(int), sizeof(int),
                           NULL, NULL) == -1);
    test_cond("hashmap new invalid",
              hashmap_new((hashmap_key_kind)99, sizeof(int), sizeof(int),
                          NULL, NULL) == NULL);
    test_cond("hashmap init stack",
              hashmap_init(&map, HASHMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                           NULL, NULL) == 0);
    test_cond("hashmap reserve stack", hashmap_reserve(&map, 8) == 0);
    test_cond("hashmap set null key", hashmap_set(&map, NULL, &(int){1}) == -1);
    test_cond("hashmap set null value", hashmap_set(&map, &(int){1}, NULL) == -1);
    test_cond("hashmap contains empty", hashmap_contains(&map, &(int){1}) == 0);
    test_cond("hashmap delete empty", hashmap_delete(&map, &(int){1}) == -1);
    hashmap_destroy(&map);

    hashmap *fail_map = hashmap_new(HASHMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                                    NULL, int_copy_with_fail);
    copy_count = 0;
    copy_fail_after = -1;
    test_cond("hashmap init fail copy", fail_map != NULL);
    test_cond("hashmap copy success", hashmap_set_value(fail_map, int, 1, int, 11) == 0);
    copy_fail_after = 1;
    test_cond("hashmap copy failure insert", hashmap_set_value(fail_map, int, 2, int, 22) == -1);
    test_cond("hashmap copy failure size", hashmap_size(fail_map) == 1);
    test_cond("hashmap copy preserve", *hashmap_get_as(fail_map, int, 1, int) == 11);
    copy_fail_after = 1;
    test_cond("hashmap copy failure update", hashmap_set_value(fail_map, int, 1, int, 33) == -1);
    test_cond("hashmap update preserve", *hashmap_get_as(fail_map, int, 1, int) == 11);
    copy_fail_after = -1;
    hashmap_free(fail_map);

    hashmap *fail_cstr = hashmap_new(HASHMAP_KEY_CSTR, sizeof(char *), sizeof(int), NULL, NULL);
    test_cond("hashmap cstr init", fail_cstr != NULL);
    test_cond("hashmap cstr null key", hashmap_set(fail_cstr, &(char *){NULL}, &(int){1}) == -1);
    hashmap_free(fail_cstr);
}

int main(void) {
    hashmap_test_int_runtime();
    hashmap_test_growth_runtime();
    hashmap_test_owned_runtime();
    hashmap_test_copy_clone();
    hashmap_test_runtime_failures();
    test_report();
}
