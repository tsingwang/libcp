#include "bitset.h"
#include "../testhelp.h"

#include <string.h>

void bitset_test(void) {
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
    test_cond("bitset", memcmp(bitset_to_string(bs, buf, n), "000001001\0", 10) == 0);
    test_cond("bitset", bitset_get(bs, 0));
    test_cond("bitset", !bitset_get(bs, 1));

    bitset_flip(bs, 5);
    test_cond("bitset", memcmp(bitset_to_string(bs, buf, n), "000101001\0", 10) == 0);

    bitset_reset(bs, 5);
    test_cond("bitset", memcmp(bitset_to_string(bs, buf, n), "000001001\0", 10) == 0);
    test_cond("bitset", bitset_count(bs) == 2);

    bitset_free(bs);
    test_report();
}

int main(void) {
    bitset_test();
}
