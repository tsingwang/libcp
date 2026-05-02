#include "deque_typed.h"
#include "testhelp.h"

void deque_test_typed_basic(void) {
    deque_uint *q = deque_new(q);
    deque_push_front(q, 0);
    deque_push_back(q, 1);
    deque_push_back(q, 2);
    deque_push_back(q, 3);
    deque_push_front(q, 4);
    deque_push_front(q, 5);
    deque_push_front(q, 6);
    deque_push_front(q, 7);
    deque_push_back(q, 8);
    test_cond("typed deque back", deque_back(q) == 8);
    test_cond("typed deque front", deque_front(q) == 7);
    deque_pop_back(q);
    deque_pop_front(q);
    test_cond("typed deque size", deque_size(q) == 7);
    test_cond("typed deque back after pop", deque_back(q) == 3);
    test_cond("typed deque front after pop", deque_front(q) == 6);

    deque_free(q);
}

void deque_test_typed_growth_back(void) {
    deque_int *q = deque_new(q);
    for (int i = 0; i < 9; i++) {
        deque_push_back(q, i);
    }

    test_cond("typed deque grow back size", deque_size(q) == 9);
    test_cond("typed deque grow back front", deque_front(q) == 0);
    test_cond("typed deque grow back back", deque_back(q) == 8);

    deque_free(q);
}

void deque_test_typed_wrap_growth(void) {
    deque_int *q = deque_new(q);
    for (int i = 0; i < 7; i++) {
        deque_push_back(q, i);
    }
    for (int i = 0; i < 3; i++) {
        deque_pop_front(q);
    }
    for (int i = 7; i < 12; i++) {
        deque_push_back(q, i);
    }

    test_cond("typed deque wrap size", deque_size(q) == 9);
    test_cond("typed deque wrap front", deque_front(q) == 3);
    test_cond("typed deque wrap back", deque_back(q) == 11);

    deque_clear(q);
    test_cond("typed deque clear", deque_empty(q));
    deque_push_front(q, 42);
    test_cond("typed deque reuse", deque_front(q) == 42 && deque_back(q) == 42);

    deque_free(q);
}

int main(void) {
    deque_test_typed_basic();
    deque_test_typed_growth_back();
    deque_test_typed_wrap_growth();
    test_report();
}
