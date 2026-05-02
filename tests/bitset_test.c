#include "bitset.h"
#include "testhelp.h"

#include <string.h>

static void bitset_zero_size_test(void) {
    char buf[1];
    bitset bs;

    test_cond("bitset init invalid", bitset_init(NULL, 8) == -1);
    test_cond("bitset zero init", bitset_init(&bs, 0) == 0);
    test_cond("bitset zero size", bitset_size(&bs) == 0);
    test_cond("bitset zero none", bitset_none(&bs));
    test_cond("bitset zero any", !bitset_any(&bs));
    test_cond("bitset zero all", bitset_all(&bs));
    test_cond("bitset zero count", bitset_count(&bs) == 0);
    test_cond("bitset zero first set", bitset_find_first_set(&bs) == BITSET_NPOS);
    test_cond("bitset zero first reset", bitset_find_first_reset(&bs) == BITSET_NPOS);
    test_cond("bitset zero string", strcmp(bitset_to_string(&bs, buf, sizeof(buf)), "") == 0);
    test_cond("bitset zero parse", bitset_from_string(&bs, "") == 0);

    bitset_destroy(&bs);
}

static void bitset_tail_bits_test(void) {
    bitset *bs = bitset_new(9);

    bitset_set(bs, 8, true);
    bitset_flip_all(bs);
    test_cond("bitset tail masked after flip", bs->bits[1] == 0x00);

    bitset_flip_all(bs);
    test_cond("bitset tail masked after flip twice", bs->bits[1] == 0x01);
    test_cond("bitset tail count", bitset_count(bs) == 1);

    bitset_free(bs);
}

static void bitset_stack_api_test(void) {
    bitset bs;
    char buf[10];

    test_cond("bitset stack init", bitset_init(&bs, 9) == 0);
    test_cond("bitset stack empty", bitset_none(&bs));

    bitset_set(&bs, 0, true);
    bitset_set(&bs, 8, true);
    test_cond("bitset stack string",
              strcmp(bitset_to_string(&bs, buf, sizeof(buf)), "100000001") == 0);

    bitset_destroy(&bs);
    test_cond("bitset stack destroy size", bs.size == 0);
    test_cond("bitset stack destroy bits", bs.bits == NULL);
}

static void bitset_bulk_ops_test(void) {
    bitset *bs = bitset_new(10);

    bitset_set_all(bs);
    test_cond("bitset set all", bitset_all(bs));
    test_cond("bitset set all count", bitset_count(bs) == 10);

    bitset_reset_all(bs);
    test_cond("bitset reset all", bitset_none(bs));

    bitset_set(bs, 4, true);
    bitset_clear(bs);
    test_cond("bitset clear", bitset_none(bs));

    bitset_free(bs);
}

static void bitset_bitwise_ops_test(void) {
    char buf[10];
    bitset *lhs = bitset_new(9);
    bitset *rhs = bitset_new(9);
    bitset *tmp;

    bitset_from_string(lhs, "101010001");
    bitset_from_string(rhs, "110000101");

    tmp = bitset_clone(lhs);
    bitset_and(tmp, rhs);
    test_cond("bitset and", strcmp(bitset_to_string(tmp, buf, sizeof(buf)), "100000001") == 0);

    bitset_copy(tmp, lhs);
    bitset_or(tmp, rhs);
    test_cond("bitset or", strcmp(bitset_to_string(tmp, buf, sizeof(buf)), "111010101") == 0);

    bitset_copy(tmp, lhs);
    bitset_xor(tmp, rhs);
    test_cond("bitset xor", strcmp(bitset_to_string(tmp, buf, sizeof(buf)), "011010100") == 0);
    test_cond("bitset equal false", !bitset_equal(lhs, rhs));

    bitset_copy(tmp, lhs);
    test_cond("bitset equal true", bitset_equal(lhs, tmp));

    bitset_free(tmp);
    bitset_free(rhs);
    bitset_free(lhs);
}

static void bitset_copy_clone_test(void) {
    char buf[10];
    bitset src;
    bitset dst;
    bitset *clone;

    test_cond("bitset copy src init", bitset_init(&src, 9) == 0);
    test_cond("bitset copy dst init", bitset_init(&dst, 1) == 0);
    bitset_from_string(&src, "001001011");
    bitset_set(&dst, 0, true);

    test_cond("bitset copy null dst", bitset_copy(NULL, &src) == -1);
    test_cond("bitset copy null src", bitset_copy(&dst, NULL) == -1);
    test_cond("bitset copy success", bitset_copy(&dst, &src) == 0);
    test_cond("bitset copy size", bitset_size(&dst) == bitset_size(&src));
    test_cond("bitset copy value", bitset_equal(&dst, &src));

    clone = bitset_clone(&src);
    test_cond("bitset clone alloc", clone != NULL);
    test_cond("bitset clone value", bitset_equal(clone, &src));
    bitset_flip(clone, 0);
    test_cond("bitset clone independent",
              strcmp(bitset_to_string(clone, buf, sizeof(buf)), "001001010") == 0);
    test_cond("bitset source unchanged",
              strcmp(bitset_to_string(&src, buf, sizeof(buf)), "001001011") == 0);

    bitset_free(clone);
    bitset_destroy(&dst);
    bitset_destroy(&src);
}

static void bitset_find_test(void) {
    bitset *bs = bitset_new(9);

    bitset_set(bs, 0, true);
    bitset_set(bs, 3, true);
    bitset_set(bs, 5, true);

    test_cond("bitset find first set", bitset_find_first_set(bs) == 0);
    test_cond("bitset find next set", bitset_find_next_set(bs, 0) == 3);
    test_cond("bitset find next set end", bitset_find_next_set(bs, 5) == BITSET_NPOS);
    test_cond("bitset find first reset", bitset_find_first_reset(bs) == 1);
    test_cond("bitset find next reset", bitset_find_next_reset(bs, 1) == 2);
    test_cond("bitset find next reset end", bitset_find_next_reset(bs, 8) == BITSET_NPOS);

    bitset_free(bs);
}

static void bitset_string_roundtrip_test(void) {
    char buf[10];
    bitset bs;

    test_cond("bitset string init", bitset_init(&bs, 9) == 0);
    test_cond("bitset parse success", bitset_from_string(&bs, "101100011") == 0);
    test_cond("bitset parse roundtrip",
              strcmp(bitset_to_string(&bs, buf, sizeof(buf)), "101100011") == 0);
    test_cond("bitset parse count", bitset_count(&bs) == 5);
    test_cond("bitset parse invalid len", bitset_from_string(&bs, "1011") == -1);
    test_cond("bitset parse keep value len",
              strcmp(bitset_to_string(&bs, buf, sizeof(buf)), "101100011") == 0);
    test_cond("bitset parse invalid char", bitset_from_string(&bs, "10110x011") == -1);
    test_cond("bitset parse keep value char",
              strcmp(bitset_to_string(&bs, buf, sizeof(buf)), "101100011") == 0);

    bitset_destroy(&bs);
}

static void bitset_basic_test(void) {
    char buf[32];
    size_t n = sizeof(buf);
    bitset *bs = bitset_new(9);
    test_cond("bitset", memcmp(bitset_to_string(bs, buf, n), "000000000\0", 10) == 0);

    test_cond("bitset", bitset_none(bs));
    test_cond("bitset", !bitset_any(bs));
    test_cond("bitset", !bitset_all(bs));
    bitset_flip_all(bs);
    test_cond("bitset", !bitset_none(bs));
    test_cond("bitset", bitset_any(bs));
    test_cond("bitset", bitset_all(bs));

    bitset_flip_all(bs);
    bitset_set(bs, 0, true);
    bitset_set(bs, 3, true);
    bitset_set(bs, 3, false);
    bitset_set(bs, 3, true);
    test_cond("bitset", memcmp(bitset_to_string(bs, buf, n), "000001001\0", 10) == 0);
    test_cond("bitset", bitset_get(bs, 0));
    test_cond("bitset", !bitset_get(bs, 1));

    bitset_flip(bs, 5);
    test_cond("bitset", memcmp(bitset_to_string(bs, buf, n), "000101001\0", 10) == 0);

    bitset_reset(bs, 5);
    test_cond("bitset", memcmp(bitset_to_string(bs, buf, n), "000001001\0", 10) == 0);
    test_cond("bitset", bitset_count(bs) == 2);

    bitset_free(bs);
}

int main(void) {
    bitset_zero_size_test();
    bitset_tail_bits_test();
    bitset_stack_api_test();
    bitset_bulk_ops_test();
    bitset_bitwise_ops_test();
    bitset_copy_clone_test();
    bitset_find_test();
    bitset_string_roundtrip_test();
    bitset_basic_test();
    test_report();
}
