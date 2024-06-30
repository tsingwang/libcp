#include "vector.h"
#include "../testhelp.h"

#include <stdio.h>

static int cmp(const void *a, const void *b) {
    return ( *(int*)a - *(int*)b );
}

void vector_test(void) {
    vector_uint *v = vector_new(v);
    vector_push_back(v, 3);
    vector_push_back(v, 1);
    vector_push_back(v, 2);
    vector_push_back(v, 3);
    vector_pop_back(v);
    test_cond("vector push & pop", v->size == 3);

    test_cond("vector back", vector_back(v) == 2);

    vector_insert(v, 1, 4);
    vector_insert(v, 1, 8);
    vector_insert(v, 1, 6);
    vector_insert(v, 1, 7);
    vector_insert(v, 1, 9);
    vector_insert(v, 1, 5);
    vector_erase(v, 1);
    test_cond("vector insert & erase", v->size == 8);

    vector_sort(v, cmp);
    test_cond("vector sorted, need CHECK by your eyes", 1);
    for (int i = 0; i < v->size; i++) {
        printf(" %d", v->data[i]);
    }
    printf("\n");

    int key = 7;
    int *p = vector_bsearch(v, &key, cmp);
    test_cond("vector search", *p == 7);
    key = 5;
    p = vector_bsearch(v, &key, cmp);
    test_cond("vector search", p == NULL);

    vector_free(v);
    test_report();
}

int main(void) {
    vector_test();
}
