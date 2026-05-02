#include "heap.h"

#include <stdlib.h>
#include <string.h>

static unsigned char *heap_slot(const heap *h, size_t index) {
    return h->data + index * h->elem_size;
}

static int heap_copy_element(const heap *h, void *dst, const void *src) {
    if (h->elem_copy != NULL) {
        return h->elem_copy(dst, src);
    }

    memcpy(dst, src, h->elem_size);
    return 0;
}

static void heap_destroy_element(const heap *h, void *elem) {
    if (h->elem_destroy != NULL) {
        h->elem_destroy(elem);
    }
}

static void heap_swap_slots(heap *h, size_t i, size_t j) {
    if (i == j) return;

    unsigned char *a = heap_slot(h, i);
    unsigned char *b = heap_slot(h, j);
    for (size_t k = 0; k < h->elem_size; k++) {
        unsigned char tmp = a[k];
        a[k] = b[k];
        b[k] = tmp;
    }
}

int heap_init(heap *h, size_t elem_size,
              heap_elem_compare_fn elem_compare,
              heap_elem_destroy_fn elem_destroy,
              heap_elem_copy_fn elem_copy) {
    if (h == NULL || elem_size == 0 || elem_compare == NULL) return -1;

    h->capacity = 0;
    h->size = 0;
    h->elem_size = elem_size;
    h->elem_compare = elem_compare;
    h->elem_destroy = elem_destroy;
    h->elem_copy = elem_copy;
    h->data = NULL;
    return 0;
}

void heap_destroy(heap *h) {
    if (h == NULL) return;

    heap_clear(h);
    free(h->data);
    h->data = NULL;
    h->capacity = 0;
}

heap *heap_new(size_t elem_size,
               heap_elem_compare_fn elem_compare,
               heap_elem_destroy_fn elem_destroy,
               heap_elem_copy_fn elem_copy) {
    heap *h = malloc(sizeof(*h));
    if (h == NULL) return NULL;
    if (heap_init(h, elem_size, elem_compare, elem_destroy, elem_copy) != 0) {
        free(h);
        return NULL;
    }
    return h;
}

void heap_free(heap *h) {
    if (h == NULL) return;

    heap_destroy(h);
    free(h);
}

int heap_copy(heap *dst, const heap *src) {
    if (dst == NULL || src == NULL) return -1;
    if (dst == src) return 0;

    heap_destroy(dst);
    if (heap_init(dst, src->elem_size, src->elem_compare,
                  src->elem_destroy, src->elem_copy) != 0) {
        return -1;
    }
    if (heap_reserve(dst, src->size) != 0) return -1;

    for (size_t i = 0; i < src->size; i++) {
        if (heap_push(dst, heap_slot(src, i)) != 0) {
            heap_clear(dst);
            return -1;
        }
    }

    return 0;
}

heap *heap_clone(const heap *src) {
    if (src == NULL) return NULL;

    heap *copy = heap_new(src->elem_size, src->elem_compare,
                          src->elem_destroy, src->elem_copy);
    if (copy == NULL) return NULL;
    if (heap_copy(copy, src) != 0) {
        heap_free(copy);
        return NULL;
    }

    return copy;
}

int heap_reserve(heap *h, size_t capacity) {
    if (h == NULL) return -1;
    if (capacity <= h->capacity) return 0;

    void *new_data = realloc(h->data, capacity * h->elem_size);
    if (new_data == NULL) return -1;

    h->data = new_data;
    h->capacity = capacity;
    return 0;
}

int heap_push(heap *h, const void *elem) {
    if (h == NULL || elem == NULL) return -1;

    if (h->size == h->capacity) {
        size_t new_capacity = h->capacity == 0 ? HEAP_INIT_CAPACITY : h->capacity * 2;
        if (heap_reserve(h, new_capacity) != 0) return -1;
    }

    unsigned char *staged = malloc(h->elem_size);
    if (staged == NULL) return -1;
    if (heap_copy_element(h, staged, elem) != 0) {
        free(staged);
        return -1;
    }

    size_t i = h->size++;
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        unsigned char *parent_slot = heap_slot(h, parent);
        if (h->elem_compare(parent_slot, staged) >= 0) break;
        memcpy(heap_slot(h, i), parent_slot, h->elem_size);
        i = parent;
    }

    memcpy(heap_slot(h, i), staged, h->elem_size);
    free(staged);
    return 0;
}

int heap_pop(heap *h) {
    if (h == NULL || heap_empty(h)) return -1;

    heap_destroy_element(h, heap_slot(h, 0));
    h->size--;
    if (h->size == 0) return 0;

    memcpy(heap_slot(h, 0), heap_slot(h, h->size), h->elem_size);

    size_t i = 0;
    while (1) {
        size_t left = i * 2 + 1;
        size_t right = left + 1;
        if (left >= h->size) break;

        size_t child = left;
        if (right < h->size &&
            h->elem_compare(heap_slot(h, left), heap_slot(h, right)) < 0) {
            child = right;
        }

        if (h->elem_compare(heap_slot(h, i), heap_slot(h, child)) >= 0) break;
        heap_swap_slots(h, i, child);
        i = child;
    }

    return 0;
}

void heap_clear(heap *h) {
    if (h == NULL) return;

    if (h->elem_destroy != NULL) {
        for (size_t i = 0; i < h->size; i++) {
            heap_destroy_element(h, heap_slot(h, i));
        }
    }

    h->size = 0;
}

void *heap_top(heap *h) {
    if (h == NULL || heap_empty(h)) return NULL;
    return heap_slot(h, 0);
}

const void *heap_top_const(const heap *h) {
    if (h == NULL || heap_empty(h)) return NULL;
    return heap_slot(h, 0);
}
