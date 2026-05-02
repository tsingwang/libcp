#include "vector.h"

#include <stdlib.h>
#include <string.h>

static int vector_copy_element(const vector *v, void *dst, const void *src) {
    if (v->elem_copy != NULL) {
        return v->elem_copy(dst, src);
    }

    memcpy(dst, src, v->elem_size);
    return 0;
}

static void vector_destroy_element(const vector *v, void *elem) {
    if (v->elem_destroy != NULL) {
        v->elem_destroy(elem);
    }
}

int vector_init(vector *v, size_t elem_size,
                vector_elem_destroy_fn elem_destroy,
                vector_elem_copy_fn elem_copy) {
    if (v == NULL || elem_size == 0) return -1;

    v->capacity = 0;
    v->size = 0;
    v->elem_size = elem_size;
    v->elem_destroy = elem_destroy;
    v->elem_copy = elem_copy;
    v->data = NULL;
    return 0;
}

void vector_destroy(vector *v) {
    if (v == NULL) return;

    vector_clear(v);
    free(v->data);
    v->data = NULL;
    v->capacity = 0;
}

vector *vector_new(size_t elem_size,
                   vector_elem_destroy_fn elem_destroy,
                   vector_elem_copy_fn elem_copy) {
    vector *v = malloc(sizeof(*v));
    if (v == NULL) return NULL;
    if (vector_init(v, elem_size, elem_destroy, elem_copy) != 0) {
        free(v);
        return NULL;
    }
    return v;
}

void vector_free(vector *v) {
    if (v == NULL) return;

    vector_destroy(v);
    free(v);
}

int vector_copy(vector *dst, const vector *src) {
    if (dst == NULL || src == NULL) return -1;
    if (dst == src) return 0;

    vector_destroy(dst);
    if (vector_init(dst, src->elem_size, src->elem_destroy, src->elem_copy) != 0) {
        return -1;
    }
    if (vector_reserve(dst, src->size) != 0) return -1;

    for (size_t i = 0; i < src->size; i++) {
        if (vector_push(dst, src->data + i * src->elem_size) != 0) {
            vector_clear(dst);
            return -1;
        }
    }

    return 0;
}

vector *vector_clone(const vector *src) {
    if (src == NULL) return NULL;

    vector *copy = vector_new(src->elem_size, src->elem_destroy, src->elem_copy);
    if (copy == NULL) return NULL;
    if (vector_copy(copy, src) != 0) {
        vector_free(copy);
        return NULL;
    }

    return copy;
}

int vector_reserve(vector *v, size_t capacity) {
    if (v == NULL) return -1;
    if (capacity <= v->capacity) return 0;

    void *new_data = realloc(v->data, capacity * v->elem_size);
    if (new_data == NULL) return -1;

    v->data = new_data;
    v->capacity = capacity;
    return 0;
}

int vector_push(vector *v, const void *elem) {
    if (v == NULL || elem == NULL) return -1;

    if (v->size == v->capacity) {
        size_t new_capacity = v->capacity == 0 ? VECTOR_INIT_CAPACITY : v->capacity * 2;
        if (vector_reserve(v, new_capacity) != 0) return -1;
    }

    void *slot = v->data + v->size * v->elem_size;
    if (vector_copy_element(v, slot, elem) != 0) return -1;

    v->size++;
    return 0;
}

int vector_pop(vector *v) {
    if (v == NULL || vector_empty(v)) return -1;

    v->size--;
    vector_destroy_element(v, v->data + v->size * v->elem_size);
    return 0;
}

int vector_insert(vector *v, size_t index, const void *elem) {
    if (v == NULL || elem == NULL || index > v->size) return -1;

    if (index == v->size) {
        return vector_push(v, elem);
    }

    if (v->size == v->capacity) {
        size_t new_capacity = v->capacity == 0 ? VECTOR_INIT_CAPACITY : v->capacity * 2;
        if (vector_reserve(v, new_capacity) != 0) return -1;
    }

    unsigned char *slot = v->data + index * v->elem_size;
    memmove(slot + v->elem_size, slot, (v->size - index) * v->elem_size);
    if (vector_copy_element(v, slot, elem) != 0) {
        memmove(slot, slot + v->elem_size, (v->size - index) * v->elem_size);
        return -1;
    }

    v->size++;
    return 0;
}

int vector_set(vector *v, size_t index, const void *elem) {
    if (v == NULL || elem == NULL || index >= v->size) return -1;

    unsigned char *replacement = malloc(v->elem_size);
    if (replacement == NULL) return -1;

    if (vector_copy_element(v, replacement, elem) != 0) {
        free(replacement);
        return -1;
    }

    unsigned char *slot = v->data + index * v->elem_size;
    vector_destroy_element(v, slot);
    memcpy(slot, replacement, v->elem_size);
    free(replacement);
    return 0;
}

void vector_erase(vector *v, size_t index) {
    if (v == NULL || index >= v->size) return;

    unsigned char *slot = v->data + index * v->elem_size;
    vector_destroy_element(v, slot);

    if (index + 1 < v->size) {
        memmove(slot, slot + v->elem_size, (v->size - index - 1) * v->elem_size);
    }

    v->size--;
}

void vector_clear(vector *v) {
    if (v == NULL) return;

    if (v->elem_destroy != NULL) {
        for (size_t i = 0; i < v->size; i++) {
            vector_destroy_element(v, v->data + i * v->elem_size);
        }
    }

    v->size = 0;
}

void *vector_at(vector *v, size_t index) {
    if (v == NULL || index >= v->size) return NULL;
    return v->data + index * v->elem_size;
}

void *vector_front(vector *v) {
    return vector_at(v, 0);
}

const void *vector_front_const(const vector *v) {
    return vector_at_const(v, 0);
}

const void *vector_at_const(const vector *v, size_t index) {
    if (v == NULL || index >= v->size) return NULL;
    return v->data + index * v->elem_size;
}

void *vector_back(vector *v) {
    if (v == NULL || vector_empty(v)) return NULL;
    return v->data + (v->size - 1) * v->elem_size;
}

const void *vector_back_const(const vector *v) {
    if (v == NULL || vector_empty(v)) return NULL;
    return v->data + (v->size - 1) * v->elem_size;
}

void vector_sort(vector *v,int (*cmp)(const void *, const void *)) {
    if (v == NULL || cmp == NULL) return;
    qsort(v->data, v->size, v->elem_size, cmp);
}

void *vector_bsearch(const vector *v, const void *key,
                     int (*cmp)(const void *, const void *)) {
    if (v == NULL || key == NULL || cmp == NULL) return NULL;
    return bsearch(key, v->data, v->size, v->elem_size, cmp);
}
