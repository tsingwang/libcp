#include "vector_typed.h"
#include "testhelp.h"

#include <stdio.h>
#include <string.h>

int cmp(const void *a, const void *b) {
    return (*(const int *)a - *(const int *)b);
}

int cmpstr(const void *a, const void *b) {
    return strcmp(*(char *const *)a, *(char *const *)b);
}

void vector_test_int_typed(void) {
    vector_int *v = vector_new(v);
    vector_push_back(v, 3);
    vector_push_back(v, 1);
    vector_push_back(v, 2);
    vector_push_back(v, 3);
    vector_pop_back(v);
    test_cond("typed vector push & pop", v->size == 3);

    test_cond("typed vector back", vector_back(v) == 2);

    vector_insert(v, 1, 4);
    vector_insert(v, 1, 8);
    vector_insert(v, 1, 6);
    vector_insert(v, 1, 7);
    vector_insert(v, 1, 9);
    vector_insert(v, 1, 5);
    vector_erase(v, 1);
    test_cond("typed vector insert & erase", v->size == 8);

    vector_sort(v, cmp);
    test_cond("typed vector sorted", v->data[0] == 1);
    for (size_t i = 0; i < v->size; i++) {
        printf(" %d", v->data[i]);
    }
    printf("\n");

    int key = 7;
    int *p = vector_bsearch(v, &key, cmp);
    test_cond("typed vector search", *p == 7);
    key = 5;
    p = vector_bsearch(v, &key, cmp);
    test_cond("typed vector search", p == NULL);

    vector_clear(v);
    test_cond("typed vector clear", vector_empty(v));
    vector_push_back(v, 42);
    test_cond("typed vector reuse", vector_back(v) == 42);

    vector_free(v);
}

void vector_test_growth_typed(void) {
    vector_int *v = vector_new(v);
    for (int i = 0; i < 20; i++) {
        vector_push_back(v, i);
    }

    test_cond("typed vector growth size", vector_size(v) == 20);
    test_cond("typed vector growth capacity", vector_capacity(v) >= 20);

    vector_erase(v, 0);
    vector_erase(v, vector_size(v) - 1);
    test_cond("typed vector erase front", v->data[0] == 1);
    test_cond("typed vector erase back", vector_back(v) == 18);

    vector_free(v);
}

void vector_test_str_typed(void) {
    vector_str *v = vector_new(v);
    vector_push_back(v, "345");
    vector_push_back(v, "15");
    vector_push_back(v, "2244");
    vector_push_back(v, "3");
    vector_pop_back(v);
    test_cond("typed vector push & pop", v->size == 3);
    test_cond("typed vector back", memcmp(vector_back(v), "2244\0", 5) == 0);

    vector_sort(v, cmpstr);
    test_cond("typed vector sorted", strcmp(v->data[0], "15") == 0);
    for (size_t i = 0; i < v->size; i++) {
        printf("%s ", v->data[i]);
    }
    printf("\n");

    vector_free(v);
}

int main(void) {
    vector_test_int_typed();
    vector_test_growth_typed();
    vector_test_str_typed();
    test_report();
}
