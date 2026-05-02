#include "heap_typed.h"
#include "testhelp.h"

static int int_cmp(const int a, const int b) {
    return a - b;
}

static void heap_typed_test_basic(void) {
    heap_int *h = NULL;
    test_cond("typed heap init", heap_new(h, int_cmp) != NULL);

    heap_push(h, 3);
    heap_push(h, 1);
    heap_push(h, 5);
    heap_push(h, 2);
    heap_push(h, 4);
    test_cond("typed heap top", heap_top(h) == 5);

    heap_pop(h);
    test_cond("typed heap pop", heap_top(h) == 4);
    heap_pop(h);
    test_cond("typed heap pop", heap_top(h) == 3);
    heap_pop(h);
    test_cond("typed heap pop", heap_top(h) == 2);
    heap_pop(h);
    test_cond("typed heap pop", heap_top(h) == 1);
    heap_pop(h);
    test_cond("typed heap empty", heap_empty(h));

    heap_free(h);
}

static void heap_typed_test_growth(void) {
    heap_int *h = NULL;
    test_cond("typed heap growth init", heap_new(h, int_cmp) != NULL);

    for (int i = 1; i <= 16; i++) {
        heap_push(h, i);
    }
    test_cond("typed heap growth size", heap_size(h) == 16);
    test_cond("typed heap growth capacity", heap_capacity(h) >= 16);

    for (int expected = 16; expected >= 1; expected--) {
        test_cond("typed heap growth order", heap_top(h) == expected);
        heap_pop(h);
    }
    test_cond("typed heap growth empty", heap_empty(h));

    heap_free(h);
}

static void heap_typed_test_right_child(void) {
    heap_int *h = NULL;
    test_cond("typed heap right init", heap_new(h, int_cmp) != NULL);

    heap_push(h, 10);
    heap_push(h, 8);
    heap_push(h, 9);
    heap_push(h, 1);
    heap_push(h, 2);
    heap_push(h, 7);

    heap_pop(h);
    test_cond("typed heap right child", heap_top(h) == 9);
    heap_pop(h);
    test_cond("typed heap single left", heap_top(h) == 8);
    heap_pop(h);
    test_cond("typed heap reuse", heap_top(h) == 7);

    heap_free(h);
}

int main(void) {
    heap_typed_test_basic();
    heap_typed_test_growth();
    heap_typed_test_right_child();
    test_report();
}
