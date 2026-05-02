#include "bitree.h"
#include "testhelp.h"

static void bitree_zero_size_test(void) {
    bitree bt;

    test_cond("bitree init invalid", bitree_init(NULL, 4) == -1);
    test_cond("bitree zero init", bitree_init(&bt, 0) == 0);
    test_cond("bitree zero size", bitree_size(&bt) == 0);
    test_cond("bitree zero prefix", bitree_prefix(&bt, 0) == 0);
    test_cond("bitree zero range", bitree_range(&bt, 0, 0) == 0);
    test_cond("bitree zero add invalid", bitree_add(&bt, 0, 1) == -1);

    bitree_destroy(&bt);
}

static void bitree_basic_test(void) {
    bitree *bt = bitree_new(6);

    test_cond("bitree new", bt != NULL);
    test_cond("bitree add", bitree_add(bt, 0, 3) == 0);
    test_cond("bitree add", bitree_add(bt, 1, 2) == 0);
    test_cond("bitree add", bitree_add(bt, 4, 7) == 0);
    test_cond("bitree add negative", bitree_add(bt, 1, -1) == 0);
    test_cond("bitree prefix 0", bitree_prefix(bt, 0) == 0);
    test_cond("bitree prefix 2", bitree_prefix(bt, 2) == 4);
    test_cond("bitree prefix full", bitree_prefix(bt, 6) == 11);
    test_cond("bitree range middle", bitree_range(bt, 1, 5) == 8);
    test_cond("bitree get", bitree_get(bt, 4) == 7);
    test_cond("bitree set", bitree_set(bt, 4, 10) == 0);
    test_cond("bitree set value", bitree_get(bt, 4) == 10);
    test_cond("bitree updated total", bitree_prefix(bt, 6) == 14);
    test_cond("bitree reset", bitree_reset(bt) == 0);
    test_cond("bitree reset total", bitree_prefix(bt, 6) == 0);

    bitree_free(bt);
}

static void bitree_copy_clone_test(void) {
    bitree src;
    bitree dst;
    bitree *clone;

    test_cond("bitree copy init src", bitree_init(&src, 5) == 0);
    test_cond("bitree copy init dst", bitree_init(&dst, 1) == 0);
    test_cond("bitree copy fill", bitree_add(&src, 0, 4) == 0);
    test_cond("bitree copy fill", bitree_add(&src, 3, 6) == 0);

    test_cond("bitree copy null dst", bitree_copy(NULL, &src) == -1);
    test_cond("bitree copy null src", bitree_copy(&dst, NULL) == -1);
    test_cond("bitree copy success", bitree_copy(&dst, &src) == 0);
    test_cond("bitree copy size", bitree_size(&dst) == bitree_size(&src));
    test_cond("bitree copy total", bitree_prefix(&dst, 5) == 10);
    test_cond("bitree copy point", bitree_get(&dst, 3) == 6);

    clone = bitree_clone(&src);
    test_cond("bitree clone alloc", clone != NULL);
    test_cond("bitree clone total", bitree_prefix(clone, 5) == 10);
    test_cond("bitree clone update", bitree_add(clone, 1, 5) == 0);
    test_cond("bitree clone independent", bitree_prefix(clone, 5) == 15);
    test_cond("bitree source unchanged", bitree_prefix(&src, 5) == 10);

    bitree_free(clone);
    bitree_destroy(&dst);
    bitree_destroy(&src);
}

static void bitree_stack_api_test(void) {
    bitree bt;

    test_cond("bitree stack init", bitree_init(&bt, 4) == 0);
    test_cond("bitree stack add", bitree_add(&bt, 2, 9) == 0);
    test_cond("bitree stack get", bitree_get(&bt, 2) == 9);

    bitree_destroy(&bt);
    test_cond("bitree stack destroy size", bt.size == 0);
    test_cond("bitree stack destroy nodes", bt.nodes == NULL);
}

static void bitree_invalid_test(void) {
    bitree bt;
    bitree *zero;

    test_cond("bitree init", bitree_init(&bt, 3) == 0);
    test_cond("bitree add invalid", bitree_add(&bt, 3, 1) == -1);
    test_cond("bitree set invalid", bitree_set(&bt, 3, 1) == -1);
    bitree_destroy(&bt);

    zero = bitree_new(0);
    test_cond("bitree new zero", zero != NULL);
    bitree_free(zero);
    test_cond("bitree clone null", bitree_clone(NULL) == NULL);
}

int main(void) {
    bitree_zero_size_test();
    bitree_basic_test();
    bitree_copy_clone_test();
    bitree_stack_api_test();
    bitree_invalid_test();
    test_report();
}
