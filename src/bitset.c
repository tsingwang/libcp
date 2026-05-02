#include "bitset.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static size_t bitset_num_bytes(size_t num_bits) {
    return num_bits == 0 ? 0 : (num_bits + CHAR_BIT - 1) / CHAR_BIT;
}

static size_t bitset_byte_index(size_t pos) {
    return pos / CHAR_BIT;
}

static unsigned char bitset_bit_mask(size_t pos) {
    return (unsigned char)((size_t)1u << (pos % CHAR_BIT));
}

static unsigned char bitset_last_byte_mask(size_t size) {
    size_t used_bits = size % CHAR_BIT;

    if (used_bits == 0) return UCHAR_MAX;
    return (unsigned char)(((size_t)1u << used_bits) - 1u);
}

static void bitset_clear_unused_tail_bits(bitset *bs) {
    size_t num_bytes = bitset_num_bytes(bs->size);

    if (num_bytes == 0 || bs->size % CHAR_BIT == 0) return;
    bs->bits[num_bytes - 1] &= bitset_last_byte_mask(bs->size);
}

static size_t bitset_popcount_byte(unsigned char byte) {
    size_t count = 0;

    while (byte != 0) {
        byte &= (unsigned char)(byte - 1);
        ++count;
    }

    return count;
}

static int bitset_resize(bitset *bs, size_t num_bits) {
    size_t num_bytes = bitset_num_bytes(num_bits);
    unsigned char *new_bits = NULL;

    assert(bs != NULL);

    size_t old_num_bytes = bitset_num_bytes(bs->size);
    if (num_bytes != 0) {
        new_bits = realloc(bs->bits, num_bytes * sizeof(*bs->bits));
        if (new_bits == NULL) return -1;
        if (num_bytes > old_num_bytes) {
            memset(new_bits + old_num_bytes, 0, num_bytes - old_num_bytes);
        }
    } else {
        free(bs->bits);
    }

    bs->bits = new_bits;
    bs->size = num_bits;
    bitset_clear_unused_tail_bits(bs);
    return 0;
}

static size_t bitset_find_from(const bitset *bs, size_t start, bool value) {
    if (start >= bs->size) return BITSET_NPOS;

    for (size_t i = start; i < bs->size; ++i) {
        if (bitset_get(bs, i) == value) return i;
    }

    return BITSET_NPOS;
}

int bitset_init(bitset *bs, size_t num_bits) {
    if (bs == NULL) return -1;

    bs->size = 0;
    bs->bits = NULL;
    return bitset_resize(bs, num_bits);
}

void bitset_destroy(bitset *bs) {
    if (bs == NULL) return;

    free(bs->bits);
    bs->bits = NULL;
    bs->size = 0;
}

bitset *bitset_new(size_t num_bits) {
    bitset *bs = malloc(sizeof(*bs));

    if (bs == NULL) return NULL;
    if (bitset_init(bs, num_bits) != 0) {
        free(bs);
        return NULL;
    }

    return bs;
}

void bitset_free(bitset *bs) {
    if (bs == NULL) return;

    bitset_destroy(bs);
    free(bs);
}

bitset *bitset_set(bitset *bs, size_t pos, bool value) {
    assert(bs != NULL && bs->size > pos);
    if (value) {
        bs->bits[bitset_byte_index(pos)] |= bitset_bit_mask(pos);
    } else {
        bs->bits[bitset_byte_index(pos)] &= (unsigned char)~bitset_bit_mask(pos);
    }
    return bs;
}

bitset *bitset_reset(bitset *bs, size_t pos) {
    assert(bs != NULL && bs->size > pos);
    bs->bits[bitset_byte_index(pos)] &= (unsigned char)~bitset_bit_mask(pos);
    return bs;
}

bitset *bitset_set_all(bitset *bs) {
    assert(bs != NULL);
    size_t num_bytes = bitset_num_bytes(bs->size);
    memset(bs->bits, UCHAR_MAX, num_bytes);
    bitset_clear_unused_tail_bits(bs);
    return bs;
}

bitset *bitset_reset_all(bitset *bs) {
    assert(bs != NULL);
    size_t num_bytes = bitset_num_bytes(bs->size);
    memset(bs->bits, 0, num_bytes);
    return bs;
}

bitset *bitset_clear(bitset *bs) {
    return bitset_reset_all(bs);
}

bitset *bitset_flip(bitset *bs, size_t pos) {
    assert(bs != NULL && bs->size > pos);
    bs->bits[bitset_byte_index(pos)] ^= bitset_bit_mask(pos);
    return bs;
}

bitset *bitset_flip_all(bitset *bs) {
    assert(bs != NULL);
    size_t num_bytes = bitset_num_bytes(bs->size);

    for (size_t i = 0; i < num_bytes; ++i) {
        bs->bits[i] = (unsigned char)~bs->bits[i];
    }

    /* Keep the unused tail bits canonical so raw storage stays predictable. */
    bitset_clear_unused_tail_bits(bs);
    return bs;
}

bitset *bitset_and(bitset *dst, const bitset *src) {
    assert(dst != NULL && src != NULL && dst->size == src->size);
    size_t num_bytes = bitset_num_bytes(dst->size);
    for (size_t i = 0; i < num_bytes; ++i) {
        dst->bits[i] &= src->bits[i];
    }
    bitset_clear_unused_tail_bits(dst);
    return dst;
}

bitset *bitset_or(bitset *dst, const bitset *src) {
    assert(dst != NULL && src != NULL && dst->size == src->size);
    size_t num_bytes = bitset_num_bytes(dst->size);
    for (size_t i = 0; i < num_bytes; ++i) {
        dst->bits[i] |= src->bits[i];
    }
    bitset_clear_unused_tail_bits(dst);
    return dst;
}

bitset *bitset_xor(bitset *dst, const bitset *src) {
    assert(dst != NULL && src != NULL && dst->size == src->size);
    size_t num_bytes = bitset_num_bytes(dst->size);
    for (size_t i = 0; i < num_bytes; ++i) {
        dst->bits[i] ^= src->bits[i];
    }
    bitset_clear_unused_tail_bits(dst);
    return dst;
}

bool bitset_equal(const bitset *a, const bitset *b) {
    assert(a != NULL && b != NULL);
    if (a->size != b->size) return false;

    size_t num_bytes = bitset_num_bytes(a->size);
    return memcmp(a->bits, b->bits, num_bytes) == 0;
}

int bitset_copy(bitset *dst, const bitset *src) {
    if (dst == NULL || src == NULL) return -1;
    if (dst == src) return 0;
    if (bitset_resize(dst, src->size) != 0) return -1;

    size_t num_bytes = bitset_num_bytes(src->size);
    if (num_bytes != 0) memcpy(dst->bits, src->bits, num_bytes);
    return 0;
}

bitset *bitset_clone(const bitset *src) {
    if (src == NULL) return NULL;

    bitset *copy = bitset_new(src->size);
    if (copy == NULL) return NULL;

    size_t num_bytes = bitset_num_bytes(src->size);
    if (num_bytes != 0) memcpy(copy->bits, src->bits, num_bytes);
    return copy;
}

bool bitset_get(const bitset *bs, size_t pos) {
    assert(bs != NULL && bs->size > pos);
    return (bs->bits[bitset_byte_index(pos)] & bitset_bit_mask(pos)) != 0;
}

bool bitset_all(const bitset *bs) {
    assert(bs != NULL);
    size_t full_bytes = bs->size / CHAR_BIT;
    size_t num_bytes = bitset_num_bytes(bs->size);

    for (size_t i = 0; i < full_bytes; ++i) {
        if (bs->bits[i] != UCHAR_MAX) return false;
    }

    if (num_bytes == full_bytes) return true;
    return (bs->bits[num_bytes - 1] & bitset_last_byte_mask(bs->size)) ==
           bitset_last_byte_mask(bs->size);
}

bool bitset_any(const bitset *bs) {
    assert(bs != NULL);
    size_t full_bytes = bs->size / CHAR_BIT;
    size_t num_bytes = bitset_num_bytes(bs->size);

    for (size_t i = 0; i < full_bytes; ++i) {
        if (bs->bits[i] != 0) return true;
    }

    if (num_bytes == full_bytes) return false;
    return (bs->bits[num_bytes - 1] & bitset_last_byte_mask(bs->size)) != 0;
}

bool bitset_none(const bitset *bs) {
    return !bitset_any(bs);
}

size_t bitset_count(const bitset *bs) {
    assert(bs != NULL);
    size_t count = 0;
    size_t full_bytes = bs->size / CHAR_BIT;
    size_t num_bytes = bitset_num_bytes(bs->size);

    for (size_t i = 0; i < full_bytes; ++i) {
        count += bitset_popcount_byte(bs->bits[i]);
    }

    if (num_bytes != full_bytes) {
        count += bitset_popcount_byte(bs->bits[num_bytes - 1] &
                                      bitset_last_byte_mask(bs->size));
    }

    return count;
}

size_t bitset_find_first_set(const bitset *bs) {
    assert(bs != NULL);
    return bitset_find_from(bs, 0, true);
}

size_t bitset_find_next_set(const bitset *bs, size_t pos) {
    assert(bs != NULL);
    if (pos >= bs->size) return BITSET_NPOS;
    return bitset_find_from(bs, pos + 1, true);
}

size_t bitset_find_first_reset(const bitset *bs) {
    assert(bs != NULL);
    return bitset_find_from(bs, 0, false);
}

size_t bitset_find_next_reset(const bitset *bs, size_t pos) {
    assert(bs != NULL);
    if (pos >= bs->size) return BITSET_NPOS;
    return bitset_find_from(bs, pos + 1, false);
}

char *bitset_to_string(const bitset *bs, char buf[], size_t n) {
    assert(bs != NULL && n > bs->size);
    for (size_t i = 0; i < bs->size; ++i) {
        buf[bs->size - 1 - i] =
            (bs->bits[bitset_byte_index(i)] & bitset_bit_mask(i)) != 0 ? '1' : '0';
    }
    buf[bs->size] = '\0';
    return buf;
}

int bitset_from_string(bitset *bs, const char *str) {
    if (bs == NULL || str == NULL) return -1;

    size_t len = strlen(str);
    if (len != bs->size) return -1;

    for (size_t i = 0; i < len; ++i) {
        if (str[i] != '0' && str[i] != '1') return -1;
    }

    bitset_reset_all(bs);
    for (size_t i = 0; i < len; ++i) {
        if (str[i] == '1') {
            size_t pos = len - 1 - i;
            bs->bits[bitset_byte_index(pos)] |= bitset_bit_mask(pos);
        }
    }

    return 0;
}
