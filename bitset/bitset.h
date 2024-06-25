#ifndef __BITSET_H
#define __BITSET_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    size_t size;
    unsigned char* bits;
} bitset;

#define bitset_size(bs) ((bs)->size)

bitset* bitset_new(size_t num_bits);
void bitset_free(bitset *bs);
bitset* bitset_set(bitset* bs, size_t pos, bool value);
bitset* bitset_reset(bitset* bs, size_t pos);
bitset* bitset_flip(bitset* bs, size_t pos);
bitset* bitset_flip_all(bitset* bs);
bool bitset_get(const bitset* bs, size_t pos);
bool bitset_all(const bitset* bs);
bool bitset_any(const bitset* bs);
bool bitset_none(const bitset* bs);
size_t bitset_count(const bitset* bs);
char* bitset_to_string(const bitset* bs, char *buf, size_t n);

#endif
