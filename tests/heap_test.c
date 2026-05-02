#include "heap.h"
#include "testhelp.h"

#include <stdlib.h>
#include <string.h>

static int destroy_count = 0;
static int copy_count = 0;
static int copy_fail_after = -1;

static int int_cmp(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

static int str_ptr_cmp(const void *a, const void *b) {
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

static void tracked_str_ptr_destroy(void *elem) {
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

void heap_test_int_runtime(void) {
    heap *h = heap_new(sizeof(int), int_cmp, NULL, NULL);
    int expected[] = {9, 8, 7, 5, 4, 3, 2, 1};

    test_cond("heap init", h != NULL);
    test_cond("heap push", heap_push_value(h, int, 3) == 0);
    test_cond("heap push", heap_push_value(h, int, 1) == 0);
    test_cond("heap push", heap_push_value(h, int, 5) == 0);
    test_cond("heap push", heap_push_value(h, int, 2) == 0);
    test_cond("heap push", heap_push_value(h, int, 4) == 0);
    test_cond("heap push", heap_push_value(h, int, 8) == 0);
    test_cond("heap push", heap_push_value(h, int, 7) == 0);
    test_cond("heap push", heap_push_value(h, int, 9) == 0);
    test_cond("heap top", *heap_top_as(h, int) == 9);
    test_cond("heap reserve", heap_reserve(h, 32) == 0);
    test_cond("heap reserve capacity", heap_capacity(h) >= 32);
    test_cond("heap preserve after reserve", *heap_top_as(h, int) == 9);
    test_cond("heap const access", *heap_top_const_as(h, int) == 9);

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
        test_cond("heap pop order", *heap_top_as(h, int) == expected[i]);
        test_cond("heap pop", heap_pop(h) == 0);
    }

    test_cond("heap empty", heap_empty(h));
    test_cond("heap top empty", heap_top(h) == NULL);
    test_cond("heap pop empty", heap_pop(h) == -1);
    test_cond("heap reuse push", heap_push_value(h, int, 6) == 0);
    heap_clear(h);
    test_cond("heap clear", heap_empty(h));

    heap_free(h);
}

void heap_test_copy_clone(void) {
    heap src;
    heap dst;
    heap *clone;

    destroy_count = 0;
    test_cond("heap copy init src", heap_init(&src, sizeof(char *), str_ptr_cmp, tracked_str_ptr_destroy, strdup_copy) == 0);
    test_cond("heap copy init dst", heap_init(&dst, sizeof(int), int_cmp, NULL, NULL) == 0);
    test_cond("heap copy fill", heap_push_value(&src, char *, "beta") == 0);
    test_cond("heap copy fill", heap_push_value(&src, char *, "alpha") == 0);
    test_cond("heap copy fill", heap_push_value(&src, char *, "gamma") == 0);

    test_cond("heap copy null dst", heap_copy(NULL, &src) == -1);
    test_cond("heap copy null src", heap_copy(&dst, NULL) == -1);
    test_cond("heap copy success", heap_copy(&dst, &src) == 0);
    test_cond("heap copy contents", strcmp(*heap_top_as(&dst, char *), "gamma") == 0);

    clone = heap_clone(&src);
    test_cond("heap clone alloc", clone != NULL);
    test_cond("heap clone contents", strcmp(*heap_top_as(clone, char *), "gamma") == 0);
    test_cond("heap clone pop", heap_pop(clone) == 0);
    test_cond("heap clone independent",
              strcmp(*heap_top_as(clone, char *), "beta") == 0 &&
              strcmp(*heap_top_as(&src, char *), "gamma") == 0);

    heap_free(clone);
    heap_destroy(&dst);
    heap_destroy(&src);
}

void heap_test_ptr_runtime(void) {
    heap *h = heap_new(sizeof(char *), str_ptr_cmp, tracked_str_ptr_destroy, strdup_copy);
    char *first = "beta";

    destroy_count = 0;
    test_cond("heap ptr init", h != NULL);
    test_cond("heap ptr push", heap_push_value(h, char *, first) == 0);
    test_cond("heap ptr push", heap_push_value(h, char *, "alpha") == 0);
    test_cond("heap ptr push", heap_push_value(h, char *, "gamma") == 0);
    test_cond("heap ptr push", heap_push_value(h, char *, "delta") == 0);
    test_cond("heap ptr copied", *heap_top_as(h, char *) != first);
    test_cond("heap ptr top", strcmp(*heap_top_as(h, char *), "gamma") == 0);
    test_cond("heap ptr pop", heap_pop(h) == 0);
    test_cond("heap ptr pop destroy", destroy_count == 1);
    test_cond("heap ptr next top", strcmp(*heap_top_as(h, char *), "delta") == 0);
    heap_clear(h);
    test_cond("heap ptr clear", heap_empty(h));
    test_cond("heap ptr clear destroy", destroy_count == 4);

    heap_free(h);
}

void heap_test_runtime_failures(void) {
    heap h;
    test_cond("heap init invalid", heap_init(NULL, sizeof(int), int_cmp, NULL, NULL) == -1);
    test_cond("heap init invalid elem", heap_init(&h, 0, int_cmp, NULL, NULL) == -1);
    test_cond("heap init invalid cmp", heap_init(&h, sizeof(int), NULL, NULL, NULL) == -1);
    test_cond("heap new invalid cmp", heap_new(sizeof(int), NULL, NULL, NULL) == NULL);
    test_cond("heap init stack", heap_init(&h, sizeof(int), int_cmp, NULL, NULL) == 0);
    test_cond("heap reserve stack", heap_reserve(&h, 4) == 0);
    test_cond("heap push null elem", heap_push(&h, NULL) == -1);
    test_cond("heap top empty stack", heap_top(&h) == NULL);
    test_cond("heap pop empty stack", heap_pop(&h) == -1);
    heap_destroy(&h);

    heap *fail_h = heap_new(sizeof(int), int_cmp, NULL, int_copy_with_fail);
    copy_count = 0;
    copy_fail_after = -1;
    test_cond("heap init fail copy", fail_h != NULL);
    test_cond("heap copy success", heap_push_value(fail_h, int, 11) == 0);
    copy_fail_after = 1;
    test_cond("heap copy failure push", heap_push_value(fail_h, int, 22) == -1);
    test_cond("heap copy failure size", heap_size(fail_h) == 1);
    test_cond("heap copy preserve", *heap_top_as(fail_h, int) == 11);
    copy_fail_after = -1;
    heap_free(fail_h);
}

int main(void) {
    heap_test_int_runtime();
    heap_test_ptr_runtime();
    heap_test_copy_clone();
    heap_test_runtime_failures();
    test_report();
}
