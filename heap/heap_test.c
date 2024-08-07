#include "heap.h"
#include "../testhelp.h"

#include <stdio.h>

int cmp(const int a, const int b) {
    return (a - b);
}

void heap_test(void) {
    heap_int *v = heap_new(v, cmp);
    heap_push(v, 3);
    heap_push(v, 1);
    heap_push(v, 5);
    heap_push(v, 2);
    heap_push(v, 4);
    test_cond("heap", v->size == 5 && heap_top(v) == 5);
    heap_pop(v);
    test_cond("heap", v->size == 4 && heap_top(v) == 4);
    heap_pop(v);
    test_cond("heap", v->size == 3 && heap_top(v) == 3);
    heap_pop(v);
    test_cond("heap", v->size == 2 && heap_top(v) == 2);
    heap_pop(v);
    test_cond("heap", v->size == 1 && heap_top(v) == 1);
    heap_pop(v);
    test_cond("heap", v->size == 0);

    heap_free(v);
    test_report();
}

int main(void) {
    heap_test();
}
