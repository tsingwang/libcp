#include "dict.h"
#include "../testhelp.h"

#include <stdio.h>

void dict_test_0(void) {
    dict_int *d = dict_new(d, DICT_HASH_MAP, DICT_MEM_STATIC, DICT_MEM_STATIC);
    dict_expand(d);
    dict_find(d, 0);
    test_cond("dict", !dict_found(d));
    dict_set(d, 0, 0);
    dict_find(d, 0);
    test_cond("dict", dict_found(d));
    dict_set(d, 5, 5);
    dict_find(d, 5);
    test_cond("dict", dict_found(d));
    test_cond("dict", dict_cur_value(d) == 5);
    dict_delete(d, 5);
    dict_find(d, 5);
    test_cond("dict", !dict_found(d));
    for (int i = 0; i < dict_capacity(d); i++) {
        printf("%d %d %d\n", i, (d)->keys[0][i], (d)->values[0][i]);
    }
    dict_free(d);
}

void dict_test_1(void) {
    dict_str *d = dict_new(d, DICT_HASH_MAP, DICT_MEM_NOFREE, DICT_MEM_NOFREE);
    dict_set(d, "ab", "12");
    dict_set(d, "cd", "34");
    dict_find(d, "abc");
    test_cond("dict", !dict_found(d));
    dict_find(d, "ab");
    test_cond("dict", dict_found(d));
    test_cond("dict", memcmp(dict_cur_value(d), "12\0", 3) == 0);
    dict_delete(d, "cd");
    dict_find(d, "cd");
    test_cond("dict", !dict_found(d));
    for (int i = 0; i < dict_capacity(d); i++) {
        printf("%d %s %s\n", i, (d)->keys[0][i], (d)->values[0][i]);
    }
    dict_free(d);
}

void dict_test_2(void) {
    dict_str *d = dict_new(d, DICT_HASH_MAP, DICT_MEM_ALLOC, DICT_MEM_NOFREE);
    dict_set(d, "ab", "12");
    dict_set(d, "cd", "34");
    dict_find(d, "abc");
    test_cond("dict", !dict_found(d));
    dict_find(d, "ab");
    test_cond("dict", dict_found(d));
    test_cond("dict", memcmp(dict_cur_value(d), "12\0", 3) == 0);
    dict_delete(d, "cd");
    dict_find(d, "cd");
    test_cond("dict", !dict_found(d));
    dict_free(d);
}

void dict_test_3(void) {
    dict_sint *d = dict_new(d, DICT_HASH_SET, DICT_MEM_ALLOC, DICT_MEM_STATIC);
    dict_set_key(d, "ab");
    dict_set_key(d, "cd");
    dict_find(d, "abc");
    test_cond("dict", !dict_found(d));
    dict_find(d, "ab");
    test_cond("dict", dict_found(d));
    dict_delete(d, "cd");
    dict_find(d, "cd");
    test_cond("dict", !dict_found(d));
    test_cond("dict", dict_size(d) == 1);
    dict_free(d);
}

int main(void) {
    dict_test_0();
    dict_test_1();
    dict_test_2();
    dict_test_3();
    test_report();
}
