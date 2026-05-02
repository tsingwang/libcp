#include "unionfind.h"
#include "testhelp.h"

static void unionfind_zero_size_test(void) {
    unionfind uf;

    test_cond("unionfind init invalid", unionfind_init(NULL, 4) == -1);
    test_cond("unionfind zero init", unionfind_init(&uf, 0) == 0);
    test_cond("unionfind zero size", unionfind_size(&uf) == 0);
    test_cond("unionfind zero set count", unionfind_set_count(&uf) == 0);
    test_cond("unionfind zero find", unionfind_find(&uf, 0) == UNIONFIND_INVALID);
    test_cond("unionfind zero find const", unionfind_find_const(&uf, 0) == UNIONFIND_INVALID);
    test_cond("unionfind zero same", unionfind_same(&uf, 0, 0) == -1);
    test_cond("unionfind zero set size", unionfind_set_size(&uf, 0) == 0);
    test_cond("unionfind zero reset", unionfind_reset(&uf) == 0);

    unionfind_destroy(&uf);
}

static void unionfind_basic_test(void) {
    unionfind *uf = unionfind_new(6);

    test_cond("unionfind new", uf != NULL);
    test_cond("unionfind initial set count", unionfind_set_count(uf) == 6);
    test_cond("unionfind initial find", unionfind_find_const(uf, 4) == 4);
    test_cond("unionfind union", unionfind_union(uf, 0, 1) == 1);
    test_cond("unionfind union", unionfind_union(uf, 2, 3) == 1);
    test_cond("unionfind union chain", unionfind_union(uf, 1, 2) == 1);
    test_cond("unionfind same merged", unionfind_same(uf, 0, 3) == 1);
    test_cond("unionfind same separate", unionfind_same(uf, 0, 4) == 0);
    test_cond("unionfind repeat union", unionfind_union(uf, 0, 3) == 0);
    test_cond("unionfind set count", unionfind_set_count(uf) == 3);
    test_cond("unionfind set size merged", unionfind_set_size(uf, 2) == 4);
    test_cond("unionfind set size singleton", unionfind_set_size(uf, 5) == 1);
    test_cond("unionfind compressed root",
              unionfind_find(uf, 3) == unionfind_find(uf, 0));

    test_cond("unionfind reset", unionfind_reset(uf) == 0);
    test_cond("unionfind reset set count", unionfind_set_count(uf) == 6);
    test_cond("unionfind reset separate", unionfind_same(uf, 0, 3) == 0);
    test_cond("unionfind reset size", unionfind_set_size(uf, 2) == 1);

    unionfind_free(uf);
}

static void unionfind_copy_clone_test(void) {
    unionfind src;
    unionfind dst;
    unionfind *clone;

    test_cond("unionfind copy init src", unionfind_init(&src, 5) == 0);
    test_cond("unionfind copy init dst", unionfind_init(&dst, 1) == 0);
    test_cond("unionfind copy fill", unionfind_union(&src, 0, 1) == 1);
    test_cond("unionfind copy fill", unionfind_union(&src, 3, 4) == 1);

    test_cond("unionfind copy null dst", unionfind_copy(NULL, &src) == -1);
    test_cond("unionfind copy null src", unionfind_copy(&dst, NULL) == -1);
    test_cond("unionfind copy success", unionfind_copy(&dst, &src) == 0);
    test_cond("unionfind copy size", unionfind_size(&dst) == unionfind_size(&src));
    test_cond("unionfind copy set count",
              unionfind_set_count(&dst) == unionfind_set_count(&src));
    test_cond("unionfind copy same",
              unionfind_same(&dst, 0, 1) == 1 && unionfind_same(&dst, 2, 4) == 0);

    clone = unionfind_clone(&src);
    test_cond("unionfind clone alloc", clone != NULL);
    test_cond("unionfind clone same", unionfind_same(clone, 3, 4) == 1);
    test_cond("unionfind clone update", unionfind_union(clone, 1, 4) == 1);
    test_cond("unionfind clone independent",
              unionfind_same(clone, 0, 4) == 1 && unionfind_same(&src, 0, 4) == 0);

    unionfind_free(clone);
    unionfind_destroy(&dst);
    unionfind_destroy(&src);
}

static void unionfind_stack_api_test(void) {
    unionfind uf;

    test_cond("unionfind stack init", unionfind_init(&uf, 4) == 0);
    test_cond("unionfind stack union", unionfind_union(&uf, 0, 1) == 1);
    test_cond("unionfind stack union", unionfind_union(&uf, 1, 2) == 1);
    test_cond("unionfind stack root const", unionfind_find_const(&uf, 2) == unionfind_find_const(&uf, 0));

    unionfind_destroy(&uf);
    test_cond("unionfind stack destroy size", uf.size == 0);
    test_cond("unionfind stack destroy set count", uf.set_count == 0);
    test_cond("unionfind stack destroy parents", uf.parents == NULL);
    test_cond("unionfind stack destroy sizes", uf.sizes == NULL);
}

static void unionfind_invalid_test(void) {
    unionfind uf;
    unionfind *zero;

    test_cond("unionfind init", unionfind_init(&uf, 3) == 0);
    test_cond("unionfind union invalid", unionfind_union(&uf, 0, 3) == -1);
    test_cond("unionfind same invalid", unionfind_same(&uf, 3, 0) == -1);
    test_cond("unionfind find invalid", unionfind_find(&uf, 3) == UNIONFIND_INVALID);
    test_cond("unionfind set size invalid", unionfind_set_size(&uf, 3) == 0);
    unionfind_destroy(&uf);

    zero = unionfind_new(0);
    test_cond("unionfind new zero", zero != NULL);
    unionfind_free(zero);
    test_cond("unionfind clone null", unionfind_clone(NULL) == NULL);
}

int main(void) {
    unionfind_zero_size_test();
    unionfind_basic_test();
    unionfind_copy_clone_test();
    unionfind_stack_api_test();
    unionfind_invalid_test();
    test_report();
}
