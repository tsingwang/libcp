#include "deque.h"
#include "testhelp.h"

#include <stdlib.h>
#include <string.h>

static int destroy_count = 0;
static int copy_count = 0;
static int copy_fail_after = -1;

int strdup_copy(void *dst, const void *src) {
    const char *value = *(char *const *)src;
    char *copy = malloc(strlen(value) + 1);
    if (copy == NULL) return -1;
    memcpy(copy, value, strlen(value) + 1);
    *(char **)dst = copy;
    return 0;
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

void deque_test_int_runtime(void) {
    deque *q = deque_new(sizeof(int), NULL, NULL);
    test_cond("deque init", q != NULL);
    test_cond("deque push front", deque_push_front_value(q, int, 0) == 0);
    test_cond("deque push back", deque_push_back_value(q, int, 1) == 0);
    test_cond("deque push back", deque_push_back_value(q, int, 2) == 0);
    test_cond("deque push back", deque_push_back_value(q, int, 3) == 0);
    test_cond("deque push front", deque_push_front_value(q, int, 4) == 0);
    test_cond("deque push front", deque_push_front_value(q, int, 5) == 0);
    test_cond("deque push front", deque_push_front_value(q, int, 6) == 0);
    test_cond("deque push front", deque_push_front_value(q, int, 7) == 0);
    test_cond("deque push back", deque_push_back_value(q, int, 8) == 0);
    test_cond("deque back", *deque_back_as(q, int) == 8);
    test_cond("deque front", *deque_front_as(q, int) == 7);

    test_cond("deque pop back", deque_pop_back(q) == 0);
    test_cond("deque pop front", deque_pop_front(q) == 0);
    test_cond("deque size", deque_size(q) == 7);
    test_cond("deque back after pop", *deque_back_as(q, int) == 3);
    test_cond("deque front after pop", *deque_front_as(q, int) == 6);
    test_cond("deque get middle", *deque_get_as(q, 3, int) == 0);
    test_cond("deque set", deque_set_value(q, 3, int, 99) == 0);
    test_cond("deque set value", *deque_get_as(q, 3, int) == 99);
    test_cond("deque reserve", deque_reserve(q, 32) == 0);
    test_cond("deque reserve capacity", deque_capacity(q) >= 32);
    test_cond("deque preserve after reserve", *deque_get_as(q, 5, int) == 2);

    deque_clear(q);
    test_cond("deque clear", deque_empty(q));
    test_cond("deque front empty", deque_front(q) == NULL);
    test_cond("deque back empty", deque_back(q) == NULL);
    test_cond("deque pop front empty", deque_pop_front(q) == -1);
    test_cond("deque pop back empty", deque_pop_back(q) == -1);

    deque_free(q);
}

void deque_test_copy_clone(void) {
    deque src;
    deque dst;
    deque *clone;

    destroy_count = 0;
    test_cond("deque copy init src", deque_init(&src, sizeof(char *), tracked_str_ptr_destroy, strdup_copy) == 0);
    test_cond("deque copy init dst", deque_init(&dst, sizeof(int), NULL, NULL) == 0);
    test_cond("deque copy fill", deque_push_back_value(&src, char *, "alpha") == 0);
    test_cond("deque copy fill", deque_push_front_value(&src, char *, "beta") == 0);

    test_cond("deque copy null dst", deque_copy(NULL, &src) == -1);
    test_cond("deque copy null src", deque_copy(&dst, NULL) == -1);
    test_cond("deque copy success", deque_copy(&dst, &src) == 0);
    test_cond("deque copy contents",
              strcmp(*deque_front_as(&dst, char *), "beta") == 0 &&
              strcmp(*deque_back_as(&dst, char *), "alpha") == 0);

    clone = deque_clone(&src);
    test_cond("deque clone alloc", clone != NULL);
    test_cond("deque clone contents", strcmp(*deque_front_as(clone, char *), "beta") == 0);
    test_cond("deque clone update", deque_set_value(clone, 1, char *, "gamma") == 0);
    test_cond("deque clone independent",
              strcmp(*deque_back_as(clone, char *), "gamma") == 0 &&
              strcmp(*deque_back_as(&src, char *), "alpha") == 0);

    deque_free(clone);
    deque_destroy(&dst);
    deque_destroy(&src);
}

void deque_test_wrap_runtime(void) {
    deque *q = deque_new(sizeof(int), NULL, NULL);
    test_cond("deque wrap init", q != NULL);
    for (int i = 0; i < 8; i++) {
        test_cond("deque wrap fill", deque_push_back_value(q, int, i) == 0);
    }
    test_cond("deque wrap grow", deque_push_back_value(q, int, 8) == 0);
    for (int i = 0; i < 4; i++) {
        test_cond("deque wrap pop front", deque_pop_front(q) == 0);
    }
    for (int i = 9; i < 16; i++) {
        test_cond("deque wrap refill", deque_push_back_value(q, int, i) == 0);
    }
    test_cond("deque wrap front", *deque_front_as(q, int) == 4);
    test_cond("deque wrap back", *deque_back_as(q, int) == 15);
    test_cond("deque wrap get", *deque_get_as(q, 5, int) == 9);

    deque_free(q);
}

void deque_test_ptr_runtime(void) {
    deque *q = deque_new(sizeof(char *), tracked_str_ptr_destroy, strdup_copy);
    destroy_count = 0;

    test_cond("deque ptr init", q != NULL);
    test_cond("deque ptr push back", deque_push_back_value(q, char *, "beta") == 0);
    test_cond("deque ptr push front", deque_push_front_value(q, char *, "alpha") == 0);
    test_cond("deque ptr push back", deque_push_back_value(q, char *, "gamma") == 0);
    test_cond("deque ptr front", strcmp(*deque_front_as(q, char *), "alpha") == 0);
    test_cond("deque ptr back", strcmp(*deque_back_as(q, char *), "gamma") == 0);
    test_cond("deque ptr set", deque_set_value(q, 1, char *, "delta") == 0);
    test_cond("deque ptr set destroy", destroy_count == 1);
    test_cond("deque ptr set copy", strcmp(*deque_get_as(q, 1, char *), "delta") == 0);
    test_cond("deque ptr pop front", deque_pop_front(q) == 0);
    test_cond("deque ptr pop destroy", destroy_count == 2);
    test_cond("deque ptr clear", (deque_clear(q), 1));
    test_cond("deque ptr clear destroy", destroy_count == 4);

    deque_free(q);
}

void deque_test_runtime_failures(void) {
    deque q;
    test_cond("deque init invalid", deque_init(NULL, sizeof(int), NULL, NULL) == -1);
    test_cond("deque init invalid elem", deque_init(&q, 0, NULL, NULL) == -1);
    test_cond("deque init stack", deque_init(&q, sizeof(int), NULL, NULL) == 0);
    test_cond("deque reserve stack", deque_reserve(&q, 4) == 0);
    test_cond("deque push back null elem", deque_push_back(&q, NULL) == -1);
    test_cond("deque push front null elem", deque_push_front(&q, NULL) == -1);
    test_cond("deque set out of range", deque_set(&q, 0, &(int){1}) == -1);
    test_cond("deque get out of range", deque_at(&q, 0) == NULL);
    deque_destroy(&q);

    deque *fail_q = deque_new(sizeof(int), NULL, int_copy_with_fail);
    copy_count = 0;
    copy_fail_after = -1;
    test_cond("deque init fail copy", fail_q != NULL);
    test_cond("deque copy success", deque_push_back_value(fail_q, int, 11) == 0);
    copy_fail_after = 1;
    test_cond("deque copy failure push back", deque_push_back_value(fail_q, int, 22) == -1);
    test_cond("deque copy failure push front", deque_push_front_value(fail_q, int, 33) == -1);
    test_cond("deque copy failure size", deque_size(fail_q) == 1);
    test_cond("deque copy preserve", *deque_front_as(fail_q, int) == 11);
    test_cond("deque copy failure set", deque_set_value(fail_q, 0, int, 44) == -1);
    test_cond("deque set preserve", *deque_front_as(fail_q, int) == 11);
    copy_fail_after = -1;
    deque_free(fail_q);
}

int main(void) {
    deque_test_int_runtime();
    deque_test_wrap_runtime();
    deque_test_ptr_runtime();
    deque_test_copy_clone();
    deque_test_runtime_failures();
    test_report();
}
