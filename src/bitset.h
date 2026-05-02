#ifndef __BITSET_H
#define __BITSET_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    size_t size;          /* logical bit count */
    unsigned char *bits;  /* packed bytes; bits beyond size are kept at 0 */
} bitset;

#define BITSET_NPOS ((size_t)-1)
#define bitset_size(bs) ((bs)->size)

/* Example:
 *   bitset_set(bs, 3, true);
 *   bitset_get(bs, 3) == true;
 */

/* Stack usage:
 *   bitset bs;
 *   bitset_init(&bs, 16);
 *   bitset_destroy(&bs);
 */
int bitset_init(bitset *bs, size_t num_bits);
void bitset_destroy(bitset *bs);

/* Heap usage:
 *   bitset *bs = bitset_new(16);
 *   bitset_free(bs);
 */
bitset *bitset_new(size_t num_bits);
void bitset_free(bitset *bs);

bitset *bitset_set(bitset *bs, size_t pos, bool value);
bitset *bitset_reset(bitset *bs, size_t pos);
bitset *bitset_set_all(bitset *bs);
bitset *bitset_reset_all(bitset *bs);
bitset *bitset_clear(bitset *bs);
bitset *bitset_flip(bitset *bs, size_t pos);
bitset *bitset_flip_all(bitset *bs);

/* Binary bitwise operators require equal bitset_size(...) on both operands. */
bitset *bitset_and(bitset *dst, const bitset *src);
bitset *bitset_or(bitset *dst, const bitset *src);
bitset *bitset_xor(bitset *dst, const bitset *src);
bool bitset_equal(const bitset *a, const bitset *b);

/* bitset_copy(...) copies into an already initialized destination. */
int bitset_copy(bitset *dst, const bitset *src);
bitset *bitset_clone(const bitset *src);

bool bitset_get(const bitset *bs, size_t pos);
bool bitset_all(const bitset *bs);
bool bitset_any(const bitset *bs);
bool bitset_none(const bitset *bs);
size_t bitset_count(const bitset *bs);
size_t bitset_find_first_set(const bitset *bs);
size_t bitset_find_next_set(const bitset *bs, size_t pos);
size_t bitset_find_first_reset(const bitset *bs);
size_t bitset_find_next_reset(const bitset *bs, size_t pos);

/* to_string() writes exactly bitset_size(bs) characters plus '\0'. */
char *bitset_to_string(const bitset *bs, char *buf, size_t n);
int bitset_from_string(bitset *bs, const char *str);

#endif
