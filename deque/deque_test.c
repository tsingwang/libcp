#include "deque.h"
#include "../testhelp.h"

#include <stdio.h>

void deque_test(void) {
    deque_uint *q = deque_new(uint);
    deque_push_back(q, 1);
    deque_push_back(q, 2);
    deque_push_back(q, 3);
    deque_push_front(q, 4);
    deque_push_front(q, 5);
    deque_pop_back(q);
    deque_pop_front(q);
    test_cond("deque", deque_size(q) == 3);
    test_cond("deque", deque_back(q) == 2);
    test_cond("deque", deque_front(q) == 4);

    deque_free(q);
    test_report();
}

int main(void) {
    deque_test();
}
