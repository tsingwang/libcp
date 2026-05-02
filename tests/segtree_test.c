#include "segtree.h"
#include "testhelp.h"

#include <limits.h>
#include <stdint.h>

typedef struct {
    int min;
    int max;
} minmax_pair;

static int segtree_sum_merge(void *dst, const void *lhs, const void *rhs) {
    *(int64_t *)dst = *(const int64_t *)lhs + *(const int64_t *)rhs;
    return 0;
}

static int segtree_minmax_merge(void *dst, const void *lhs, const void *rhs) {
    const minmax_pair *left = lhs;
    const minmax_pair *right = rhs;
    minmax_pair *out = dst;

    out->min = left->min < right->min ? left->min : right->min;
    out->max = left->max > right->max ? left->max : right->max;
    return 0;
}

static void segtree_zero_size_test(void) {
    segtree st;
    int64_t zero = 0;
    int64_t out = -1;

    test_cond("segtree init invalid tree",
              segtree_init(NULL, 4, sizeof(int64_t), &zero, segtree_sum_merge) == -1);
    test_cond("segtree init invalid elem size",
              segtree_init(&st, 4, 0, &zero, segtree_sum_merge) == -1);
    test_cond("segtree init invalid identity",
              segtree_init(&st, 4, sizeof(int64_t), NULL, segtree_sum_merge) == -1);
    test_cond("segtree init invalid merge",
              segtree_init(&st, 4, sizeof(int64_t), &zero, NULL) == -1);
    test_cond("segtree zero init",
              segtree_init(&st, 0, sizeof(int64_t), &zero, segtree_sum_merge) == 0);
    test_cond("segtree zero size", segtree_size(&st) == 0);
    test_cond("segtree zero capacity", segtree_capacity(&st) == 1);
    test_cond("segtree zero query", segtree_query(&st, 0, 0, &out) == 0);
    test_cond("segtree zero query value", out == 0);
    test_cond("segtree zero build", segtree_build(&st, NULL) == 0);
    test_cond("segtree zero set invalid", segtree_set(&st, 0, &zero) == -1);
    test_cond("segtree zero get invalid", segtree_get(&st, 0, &out) == -1);

    segtree_destroy(&st);
}

static void segtree_basic_test(void) {
    int64_t zero = 0;
    int64_t values[] = { 3, 1, 4, 1, 5, 9 };
    int64_t out = 0;
    segtree *st = segtree_new(6, sizeof(int64_t), &zero, segtree_sum_merge);

    test_cond("segtree new", st != NULL);
    test_cond("segtree build", segtree_build(st, values) == 0);
    test_cond("segtree query full", segtree_query(st, 0, 6, &out) == 0 && out == 23);
    test_cond("segtree query middle", segtree_query(st, 1, 4, &out) == 0 && out == 6);
    test_cond("segtree get", segtree_get(st, 2, &out) == 0 && out == 4);
    test_cond("segtree set", segtree_set_value(st, 3, int64_t, 7) == 0);
    test_cond("segtree set value", segtree_get(st, 3, &out) == 0 && out == 7);
    test_cond("segtree updated range", segtree_query(st, 1, 4, &out) == 0 && out == 12);
    test_cond("segtree empty range", segtree_query(st, 3, 3, &out) == 0 && out == 0);
    test_cond("segtree reset", segtree_reset(st) == 0);
    test_cond("segtree reset total", segtree_query(st, 0, 6, &out) == 0 && out == 0);

    segtree_free(st);
}

static void segtree_copy_clone_test(void) {
    int64_t zero = 0;
    int64_t values[] = { 2, 7, 1, 8, 2 };
    int64_t out = 0;
    segtree src;
    segtree dst;
    segtree *clone;

    test_cond("segtree copy init src",
              segtree_init(&src, 5, sizeof(int64_t), &zero, segtree_sum_merge) == 0);
    test_cond("segtree copy init dst",
              segtree_init(&dst, 1, sizeof(int64_t), &zero, segtree_sum_merge) == 0);
    test_cond("segtree copy build", segtree_build(&src, values) == 0);
    test_cond("segtree copy null dst", segtree_copy(NULL, &src) == -1);
    test_cond("segtree copy null src", segtree_copy(&dst, NULL) == -1);
    test_cond("segtree copy success", segtree_copy(&dst, &src) == 0);
    test_cond("segtree copy size", segtree_size(&dst) == segtree_size(&src));
    test_cond("segtree copy total", segtree_query(&dst, 0, 5, &out) == 0 && out == 20);

    clone = segtree_clone(&src);
    test_cond("segtree clone alloc", clone != NULL);
    test_cond("segtree clone total", segtree_query(clone, 0, 5, &out) == 0 && out == 20);
    test_cond("segtree clone update", segtree_set_value(clone, 1, int64_t, 10) == 0);
    test_cond("segtree clone independent",
              segtree_query(clone, 0, 5, &out) == 0 && out == 23);
    test_cond("segtree source unchanged",
              segtree_query(&src, 0, 5, &out) == 0 && out == 20);

    segtree_free(clone);
    segtree_destroy(&dst);
    segtree_destroy(&src);
}

static void segtree_generic_merge_test(void) {
    minmax_pair identity = { INT_MAX, INT_MIN };
    minmax_pair leaves[] = {
        { 5, 5 },
        { -2, -2 },
        { 9, 9 },
        { 1, 1 },
    };
    minmax_pair out = { 0, 0 };
    segtree st;

    test_cond("segtree generic init",
              segtree_init(&st, 4, sizeof(minmax_pair),
                           &identity, segtree_minmax_merge) == 0);
    test_cond("segtree generic build", segtree_build(&st, leaves) == 0);
    test_cond("segtree generic query",
              segtree_query(&st, 1, 4, &out) == 0 &&
              out.min == -2 && out.max == 9);
    test_cond("segtree generic set",
              segtree_set(&st, 2, &(minmax_pair){ 4, 4 }) == 0);
    test_cond("segtree generic updated query",
              segtree_query(&st, 1, 4, &out) == 0 &&
              out.min == -2 && out.max == 4);

    segtree_destroy(&st);
}

static void segtree_stack_api_test(void) {
    int64_t zero = 0;
    int64_t out = 0;
    segtree st;

    test_cond("segtree stack init",
              segtree_init(&st, 4, sizeof(int64_t), &zero, segtree_sum_merge) == 0);
    test_cond("segtree stack set", segtree_set_value(&st, 2, int64_t, 9) == 0);
    test_cond("segtree stack get", segtree_get(&st, 2, &out) == 0 && out == 9);

    segtree_destroy(&st);
    test_cond("segtree stack destroy size", st.size == 0);
    test_cond("segtree stack destroy nodes", st.nodes == NULL);
    test_cond("segtree stack destroy identity", st.identity == NULL);
}

static void segtree_invalid_test(void) {
    int64_t zero = 0;
    int64_t values[] = { 1, 2, 3 };
    int64_t out = 0;
    segtree st;

    test_cond("segtree invalid init",
              segtree_init(&st, 3, sizeof(int64_t), &zero, segtree_sum_merge) == 0);
    test_cond("segtree invalid build null", segtree_build(&st, NULL) == -1);
    test_cond("segtree invalid build", segtree_build(&st, values) == 0);
    test_cond("segtree invalid set", segtree_set(&st, 3, &zero) == -1);
    test_cond("segtree invalid get", segtree_get(&st, 3, &out) == -1);
    test_cond("segtree invalid query order", segtree_query(&st, 2, 1, &out) == -1);
    test_cond("segtree invalid query bound", segtree_query(&st, 0, 4, &out) == -1);
    test_cond("segtree invalid query out", segtree_query(&st, 0, 1, NULL) == -1);

    segtree_destroy(&st);
    test_cond("segtree clone null", segtree_clone(NULL) == NULL);
}

int main(void) {
    segtree_zero_size_test();
    segtree_basic_test();
    segtree_copy_clone_test();
    segtree_generic_merge_test();
    segtree_stack_api_test();
    segtree_invalid_test();
    test_report();
}
