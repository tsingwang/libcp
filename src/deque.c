#include "deque.h"

#include <stdlib.h>
#include <string.h>

static int deque_copy_element(const deque *q, void *dst, const void *src) {
    if (q->elem_copy != NULL) {
        return q->elem_copy(dst, src);
    }

    memcpy(dst, src, q->elem_size);
    return 0;
}

static void deque_destroy_element(const deque *q, void *elem) {
    if (q->elem_destroy != NULL) {
        q->elem_destroy(elem);
    }
}

static size_t deque_slot(const deque *q, size_t index) {
    return (q->head + index) % q->capacity;
}

int deque_init(deque *q, size_t elem_size,
               deque_elem_destroy_fn elem_destroy,
               deque_elem_copy_fn elem_copy) {
    if (q == NULL || elem_size == 0) return -1;

    q->capacity = 0;
    q->size = 0;
    q->head = 0;
    q->elem_size = elem_size;
    q->elem_destroy = elem_destroy;
    q->elem_copy = elem_copy;
    q->data = NULL;
    return 0;
}

void deque_destroy(deque *q) {
    if (q == NULL) return;

    deque_clear(q);
    free(q->data);
    q->data = NULL;
    q->capacity = 0;
    q->head = 0;
}

deque *deque_new(size_t elem_size,
                 deque_elem_destroy_fn elem_destroy,
                 deque_elem_copy_fn elem_copy) {
    deque *q = malloc(sizeof(*q));
    if (q == NULL) return NULL;
    if (deque_init(q, elem_size, elem_destroy, elem_copy) != 0) {
        free(q);
        return NULL;
    }
    return q;
}

void deque_free(deque *q) {
    if (q == NULL) return;

    deque_destroy(q);
    free(q);
}

int deque_copy(deque *dst, const deque *src) {
    if (dst == NULL || src == NULL) return -1;
    if (dst == src) return 0;

    deque_destroy(dst);
    if (deque_init(dst, src->elem_size, src->elem_destroy, src->elem_copy) != 0) {
        return -1;
    }
    if (deque_reserve(dst, src->size) != 0) return -1;

    for (size_t i = 0; i < src->size; i++) {
        if (deque_push_back(dst, src->data + deque_slot(src, i) * src->elem_size) != 0) {
            deque_clear(dst);
            return -1;
        }
    }

    return 0;
}

deque *deque_clone(const deque *src) {
    if (src == NULL) return NULL;

    deque *copy = deque_new(src->elem_size, src->elem_destroy, src->elem_copy);
    if (copy == NULL) return NULL;
    if (deque_copy(copy, src) != 0) {
        deque_free(copy);
        return NULL;
    }

    return copy;
}

int deque_reserve(deque *q, size_t capacity) {
    if (q == NULL) return -1;
    if (capacity <= q->capacity) return 0;

    unsigned char *new_data = malloc(capacity * q->elem_size);
    if (new_data == NULL) return -1;

    for (size_t i = 0; i < q->size; i++) {
        size_t slot = q->capacity == 0 ? 0 : deque_slot(q, i);
        memcpy(new_data + i * q->elem_size,
               q->data + slot * q->elem_size,
               q->elem_size);
    }

    free(q->data);
    q->data = new_data;
    q->capacity = capacity;
    q->head = 0;
    return 0;
}

int deque_push_back(deque *q, const void *elem) {
    if (q == NULL || elem == NULL) return -1;

    if (q->size == q->capacity) {
        size_t new_capacity = q->capacity == 0 ? DEQUE_INIT_CAPACITY : q->capacity * 2;
        if (deque_reserve(q, new_capacity) != 0) return -1;
    }

    size_t slot = deque_slot(q, q->size);
    if (deque_copy_element(q, q->data + slot * q->elem_size, elem) != 0) return -1;

    q->size++;
    return 0;
}

int deque_push_front(deque *q, const void *elem) {
    if (q == NULL || elem == NULL) return -1;

    if (q->size == q->capacity) {
        size_t new_capacity = q->capacity == 0 ? DEQUE_INIT_CAPACITY : q->capacity * 2;
        if (deque_reserve(q, new_capacity) != 0) return -1;
    }

    if (q->size > 0) {
        q->head = q->head == 0 ? q->capacity - 1 : q->head - 1;
    }
    if (deque_copy_element(q, q->data + q->head * q->elem_size, elem) != 0) {
        if (q->size > 0) {
            q->head = (q->head + 1) % q->capacity;
        }
        return -1;
    }

    q->size++;
    return 0;
}

int deque_pop_back(deque *q) {
    if (q == NULL || deque_empty(q)) return -1;

    size_t slot = deque_slot(q, q->size - 1);
    deque_destroy_element(q, q->data + slot * q->elem_size);
    q->size--;
    if (q->size == 0) q->head = 0;
    return 0;
}

int deque_pop_front(deque *q) {
    if (q == NULL || deque_empty(q)) return -1;

    deque_destroy_element(q, q->data + q->head * q->elem_size);
    q->size--;
    if (q->size == 0) {
        q->head = 0;
    } else {
        q->head = (q->head + 1) % q->capacity;
    }
    return 0;
}

int deque_set(deque *q, size_t index, const void *elem) {
    if (q == NULL || elem == NULL || index >= q->size) return -1;

    unsigned char *replacement = malloc(q->elem_size);
    if (replacement == NULL) return -1;
    if (deque_copy_element(q, replacement, elem) != 0) {
        free(replacement);
        return -1;
    }

    size_t slot = deque_slot(q, index);
    unsigned char *target = q->data + slot * q->elem_size;
    deque_destroy_element(q, target);
    memcpy(target, replacement, q->elem_size);
    free(replacement);
    return 0;
}

void deque_clear(deque *q) {
    if (q == NULL) return;

    if (q->elem_destroy != NULL) {
        for (size_t i = 0; i < q->size; i++) {
            size_t slot = deque_slot(q, i);
            deque_destroy_element(q, q->data + slot * q->elem_size);
        }
    }

    q->size = 0;
    q->head = 0;
}

void *deque_at(deque *q, size_t index) {
    if (q == NULL || index >= q->size) return NULL;
    return q->data + deque_slot(q, index) * q->elem_size;
}

const void *deque_at_const(const deque *q, size_t index) {
    if (q == NULL || index >= q->size) return NULL;
    return q->data + deque_slot(q, index) * q->elem_size;
}

void *deque_front(deque *q) {
    return deque_at(q, 0);
}

const void *deque_front_const(const deque *q) {
    return deque_at_const(q, 0);
}

void *deque_back(deque *q) {
    if (q == NULL || deque_empty(q)) return NULL;
    return q->data + deque_slot(q, q->size - 1) * q->elem_size;
}

const void *deque_back_const(const deque *q) {
    if (q == NULL || deque_empty(q)) return NULL;
    return q->data + deque_slot(q, q->size - 1) * q->elem_size;
}
