#include "bitset.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

bitset* bitset_new(size_t num_bits) {
    bitset* bs = malloc(sizeof(*bs));
    if (bs == NULL) return NULL;

    size_t num_bytes = (num_bits + 7) / 8;
    bs->bits = calloc(1, num_bytes);
    if (bs->bits == NULL) exit(1);
    bs->size = num_bits;
    return bs;
}

void bitset_free(bitset *bs) {
    if (bs == NULL) return;
    free(bs->bits);
    free(bs);
}

bitset* bitset_set(bitset* bs, size_t pos, bool value) {
    assert(bs != NULL && bs->size > pos);
    if (value) {
        bs->bits[pos/8] |= (1 << (pos%8));
    } else {
        bs->bits[pos/8] &= ~(1 << (pos%8));
    }
    return bs;
}

bitset* bitset_reset(bitset* bs, size_t pos) {
    assert(bs != NULL && bs->size > pos);
    bs->bits[pos/8] &= ~(1 << (pos%8));
    return bs;
}

bitset* bitset_flip(bitset* bs, size_t pos) {
    assert(bs != NULL && bs->size > pos);
    bs->bits[pos/8] ^= (1 << (pos%8));
    return bs;
}

bitset* bitset_flip_all(bitset* bs) {
    assert(bs != NULL);
    size_t num_bytes = (bs->size + 7) / 8;
    for (size_t i = 0; i < num_bytes; ++i) {
        bs->bits[i] = ~(bs->bits[i]);
    }
    return bs;
}

bool bitset_get(const bitset *bs, size_t pos) {
    assert(bs != NULL && bs->size > pos);
    return (bs->bits[pos/8] & (1 << (pos%8))) != 0;
}

bool bitset_all(const bitset* bs) {
    assert(bs != NULL);
    size_t n = bs->size / 8;
    for (size_t i = 0; i < n; ++i) {
        if (bs->bits[i] != 255) {
            return false;
        }
    }
    for (size_t i = n*8; i < bs->size; ++i) {
        if (!(bs->bits[i/8] & (1 << (i%8)))) {
            return false;
        }
    }
    return true;
}

bool bitset_any(const bitset* bs) {
    assert(bs != NULL);
    size_t n = bs->size / 8;
    for (size_t i = 0; i < n; ++i) {
        if (bs->bits[i] != 0) {
            return true;
        }
    }
    for (size_t i = n*8; i < bs->size; ++i) {
        if (bs->bits[i/8] & (1 << (i%8))) {
            return true;
        }
    }
    return false;
}

bool bitset_none(const bitset* bs) {
    return !bitset_any(bs);
}

size_t bitset_count(const bitset* bs) {
    assert(bs != NULL);
    size_t count = 0;
    for (size_t i = 0; i < bs->size; ++i) {
        if (bs->bits[i/8] & (1 << (i%8))) {
            ++count;
        }
    }
    return count;
}

char* bitset_to_string(const bitset* bs, char buf[], size_t n) {
    assert(bs != NULL && n > bs->size);
    for (size_t i = 0; i < bs->size; ++i) {
        buf[bs->size - 1 - i] = bitset_get(bs, i) ? '1' : '0';
    }
    buf[bs->size] = '\0';
    return buf;
}
