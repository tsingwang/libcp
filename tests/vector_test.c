#include "vector.h"
#include "testhelp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int destroy_count = 0;
static int copy_count = 0;
static int copy_fail_after = -1;

/* For pointer elements, elem_copy decides whether the vector owns pointed
 * resources. This hook deep-copies a string into the destination slot. */
int cmp(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

int str_ptr_cmp(const void *a, const void *b) {
    return strcmp(*(char *const *)a, *(char *const *)b);
}

int strdup_copy(void *dst, const void *src) {
    const char *value = *(char *const *)src;
    char *copy = malloc(strlen(value) + 1);
    if (copy == NULL) return -1;
    memcpy(copy, value, strlen(value) + 1);
    *(char **)dst = copy;
    return 0;
}

void str_ptr_destroy(void *elem) {
    free(*(char **)elem);
}

void tracked_str_ptr_destroy(void *elem) {
    destroy_count++;
    free(*(char **)elem);
}

int int_copy_with_fail(void *dst, const void *src) {
    if (copy_fail_after >= 0 && copy_count >= copy_fail_after) {
        return -1;
    }

    copy_count++;
    memcpy(dst, src, sizeof(int));
    return 0;
}

/* Heap-allocated vector holding plain ints.
 * - vector_push(v, &x) is the low-level form
 * - vector_push_value(v, int, x) is the convenient value form
 * For scalar types, elem_copy/elem_destroy can be NULL. */
void vector_test_int_runtime(void) {
    vector *v = vector_new(sizeof(int), NULL, NULL);
    test_cond("vector init", v != NULL);
    test_cond("vector push", vector_push_value(v, int, 3) == 0);
    test_cond("vector push", vector_push_value(v, int, 1) == 0);
    test_cond("vector push", vector_push_value(v, int, 2) == 0);
    test_cond("vector push", vector_push_value(v, int, 3) == 0);
    test_cond("vector pop", vector_pop(v) == 0);
    test_cond("vector push & pop", vector_size(v) == 3);

    test_cond("vector front", *vector_front_as(v, int) == 3);
    test_cond("vector back", *vector_back_as(v, int) == 2);

    test_cond("vector insert", vector_insert_value(v, 1, int, 4) == 0);
    test_cond("vector insert", vector_insert_value(v, 1, int, 8) == 0);
    test_cond("vector insert", vector_insert_value(v, 1, int, 6) == 0);
    test_cond("vector insert", vector_insert_value(v, 1, int, 7) == 0);
    test_cond("vector insert", vector_insert_value(v, 1, int, 9) == 0);
    test_cond("vector insert", vector_insert_value(v, 1, int, 5) == 0);
    vector_erase(v, 1);
    test_cond("vector insert & erase", vector_size(v) == 8);

    vector_sort(v, cmp);
    test_cond("vector sorted", *vector_get_as(v, 0, int) == 1);
    for (size_t i = 0; i < vector_size(v); i++) {
        printf(" %d", *vector_get_as(v, i, int));
    }
    printf("\n");

    int key = 7;
    int *p = vector_bsearch(v, &key, cmp);
    test_cond("vector search", *p == 7);
    key = 5;
    p = vector_bsearch(v, &key, cmp);
    test_cond("vector search", p == NULL);

    test_cond("vector reserve", vector_reserve(v, 32) == 0);
    test_cond("vector reserve capacity", vector_capacity(v) >= 32);
    test_cond("vector preserve after reserve", *vector_get_as(v, 5, int) == 7);
    test_cond("vector insert tail", vector_insert_value(v, vector_size(v), int, 10) == 0);
    test_cond("vector tail value", *vector_back_as(v, int) == 10);
    /* vector_set is safer than writing through vector_get_as(...) when the
     * element type may need replacement hooks in the future. */
    test_cond("vector set", vector_set_value(v, 1, int, 99) == 0);
    test_cond("vector set value", *vector_get_as(v, 1, int) == 99);
    test_cond("vector const access", *vector_get_const_as(v, 0, int) == 1);
    test_cond("vector out of range", vector_at(v, vector_size(v)) == NULL);

    vector_clear(v);
    test_cond("vector clear", vector_empty(v));
    test_cond("vector back empty", vector_back(v) == NULL);
    test_cond("vector pop empty", vector_pop(v) == -1);

    vector_free(v);
}

void vector_test_copy_clone(void) {
    vector src;
    vector dst;
    vector *clone;
    char *first = "345";

    destroy_count = 0;
    test_cond("vector copy init src", vector_init(&src, sizeof(char *), tracked_str_ptr_destroy, strdup_copy) == 0);
    test_cond("vector copy init dst", vector_init(&dst, sizeof(int), NULL, NULL) == 0);
    test_cond("vector copy fill", vector_push_value(&src, char *, first) == 0);
    test_cond("vector copy fill", vector_push_value(&src, char *, "15") == 0);

    test_cond("vector copy null dst", vector_copy(NULL, &src) == -1);
    test_cond("vector copy null src", vector_copy(&dst, NULL) == -1);
    test_cond("vector copy success", vector_copy(&dst, &src) == 0);
    test_cond("vector copy contents",
              strcmp(*vector_get_as(&dst, 0, char *), "345") == 0 &&
              strcmp(*vector_get_as(&dst, 1, char *), "15") == 0);

    clone = vector_clone(&src);
    test_cond("vector clone alloc", clone != NULL);
    test_cond("vector clone contents", strcmp(*vector_front_as(clone, char *), "345") == 0);
    test_cond("vector clone update", vector_set_value(clone, 1, char *, "99") == 0);
    test_cond("vector clone independent",
              strcmp(*vector_get_as(clone, 1, char *), "99") == 0 &&
              strcmp(*vector_get_as(&src, 1, char *), "15") == 0);

    vector_free(clone);
    vector_destroy(&dst);
    vector_destroy(&src);
}

/* Heap-allocated vector holding char* elements with ownership.
 * The vector stores pointer values, but elem_copy/elem_destroy make it own the
 * pointed strings, so erase/pop/set/clear/free all release replaced elements. */
void vector_test_ptr_runtime(void) {
    vector *v = vector_new(sizeof(char *), tracked_str_ptr_destroy, strdup_copy);
    char *first = "345";
    destroy_count = 0;

    test_cond("vector init ptr", v != NULL);
    test_cond("vector push ptr", vector_push_value(v, char *, first) == 0);
    test_cond("vector push ptr", vector_push_value(v, char *, "15") == 0);
    test_cond("vector push ptr", vector_push_value(v, char *, "2244") == 0);
    test_cond("vector push ptr", vector_push_value(v, char *, "3") == 0);
    test_cond("vector copied ptr", *vector_get_as(v, 0, char *) != first);

    vector_erase(v, 1);
    test_cond("vector erase ptr", vector_size(v) == 3);
    test_cond("vector erase destroy", destroy_count == 1);

    test_cond("vector insert ptr tail", vector_insert_value(v, vector_size(v), char *, "88") == 0);
    test_cond("vector pop ptr", vector_pop(v) == 0);
    test_cond("vector pop ptr destroy", destroy_count == 2);
    test_cond("vector push & pop ptr", vector_size(v) == 3);
    test_cond("vector back ptr", strcmp(*vector_back_as(v, char *), "3") == 0);
    test_cond("vector set ptr", vector_set_value(v, 1, char *, "111") == 0);
    test_cond("vector set ptr destroy", destroy_count == 3);
    test_cond("vector set ptr copy", strcmp(*vector_get_as(v, 1, char *), "111") == 0);

    vector_sort(v, str_ptr_cmp);
    test_cond("vector sorted ptr", strcmp(*vector_get_as(v, 0, char *), "111") == 0);
    for (size_t i = 0; i < vector_size(v); i++) {
        printf("%s ", *vector_get_as(v, i, char *));
    }
    printf("\n");

    char *key = "111";
    char **match = vector_bsearch(v, &key, str_ptr_cmp);
    test_cond("vector search ptr", match != NULL && strcmp(*match, "111") == 0);

    vector_clear(v);
    test_cond("vector clear ptr", vector_empty(v));
    test_cond("vector clear ptr destroy", destroy_count == 6);

    vector_free(v);
}

/* Stack-allocated vector uses vector_init/vector_destroy instead of
 * vector_new/vector_free. These checks also document failure behavior. */
void vector_test_runtime_failures(void) {
    vector v;
    test_cond("vector init invalid", vector_init(NULL, sizeof(int), NULL, NULL) == -1);
    test_cond("vector init invalid elem", vector_init(&v, 0, NULL, NULL) == -1);
    test_cond("vector init stack", vector_init(&v, sizeof(int), NULL, NULL) == 0);
    test_cond("vector reserve stack", vector_reserve(&v, 4) == 0);
    test_cond("vector push null elem", vector_push(&v, NULL) == -1);
    test_cond("vector insert out of range", vector_insert(&v, 1, &(int){1}) == -1);
    test_cond("vector set out of range", vector_set(&v, 0, &(int){1}) == -1);
    test_cond("vector bsearch invalid", vector_bsearch(NULL, &(int){1}, cmp) == NULL);
    vector_destroy(&v);

    vector *fail_vec = vector_new(sizeof(int), NULL, int_copy_with_fail);
    copy_count = 0;
    copy_fail_after = -1;
    test_cond("vector init fail copy", fail_vec != NULL);
    test_cond("vector copy success", vector_push_value(fail_vec, int, 11) == 0);
    copy_fail_after = 1;
    test_cond("vector copy failure push", vector_push_value(fail_vec, int, 22) == -1);
    test_cond("vector copy failure size", vector_size(fail_vec) == 1);
    test_cond("vector copy preserve", *vector_get_as(fail_vec, 0, int) == 11);
    test_cond("vector copy failure insert", vector_insert_value(fail_vec, 0, int, 33) == -1);
    test_cond("vector insert preserve", *vector_get_as(fail_vec, 0, int) == 11);
    test_cond("vector copy failure set", vector_set_value(fail_vec, 0, int, 44) == -1);
    test_cond("vector set preserve", *vector_get_as(fail_vec, 0, int) == 11);
    copy_fail_after = -1;
    vector_free(fail_vec);
}

int main(void) {
    vector_test_int_runtime();
    vector_test_ptr_runtime();
    vector_test_copy_clone();
    vector_test_runtime_failures();
    test_report();
}
