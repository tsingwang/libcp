#include "dict_typed.h"
#include "testhelp.h"

#include <stdlib.h>
#include <string.h>

static char *test_strdup(const char *s) {
    char *t = malloc(strlen(s) + 1);
    if (t == NULL) return NULL;
    memcpy(t, s, strlen(s) + 1);
    return t;
}

static int int_cmp(const int a, const int b) {
    return a - b;
}

static int str_cmp(const char *a, const char *b) {
    return strcmp(a, b);
}

static void dict_typed_test_int_map(void) {
    dict_int *d = dict_new_map(d, int_cmp, NULL, NULL);
    test_cond("typed dict int init", d != NULL);
    dict_expand(d);
    dict_find(d, 0);
    test_cond("typed dict int missing", !dict_found(d));
    dict_set(d, 0, 0);
    dict_set(d, 5, 5);
    test_cond("typed dict int size", dict_size(d) == 2);
    dict_find(d, 5);
    test_cond("typed dict int found", dict_found(d));
    test_cond("typed dict int value", dict_cur_value(d) == 5);
    dict_delete(d, 5);
    dict_find(d, 5);
    test_cond("typed dict int delete", !dict_found(d));

    dict_free(d);
}

static void dict_typed_test_str_map(void) {
    dict_str *d = dict_new_map(d, str_cmp, NULL, NULL);
    test_cond("typed dict str init", d != NULL);
    dict_set(d, "ab", "12");
    dict_set(d, "cd", "34");
    dict_find(d, "abc");
    test_cond("typed dict str missing", !dict_found(d));
    dict_find(d, "ab");
    test_cond("typed dict str found", dict_found(d));
    test_cond("typed dict str value", memcmp(dict_cur_value(d), "12\0", 3) == 0);
    dict_delete(d, "cd");
    dict_find(d, "cd");
    test_cond("typed dict str delete", !dict_found(d));

    dict_free(d);
}

static void dict_typed_test_owned_map(void) {
    dict_str *d = dict_new_map(d, str_cmp, free, free);
    test_cond("typed dict owned init", d != NULL);
    dict_set(d, test_strdup("ab"), test_strdup("12"));
    dict_set(d, test_strdup("cd"), test_strdup("34"));
    dict_set(d, test_strdup("ab"), test_strdup("56"));
    dict_find(d, "ab");
    test_cond("typed dict owned update", dict_found(d));
    test_cond("typed dict owned value", memcmp(dict_cur_value(d), "56\0", 3) == 0);
    dict_delete(d, "cd");
    dict_find(d, "cd");
    test_cond("typed dict owned delete", !dict_found(d));

    dict_free(d);
}

static void dict_typed_test_set(void) {
    dict_sint *d = dict_new_set(d, str_cmp, free);
    test_cond("typed dict set init", d != NULL);
    dict_set_key(d, test_strdup("ab"));
    dict_set_key(d, test_strdup("cd"));
    dict_find(d, "abc");
    test_cond("typed dict set missing", !dict_found(d));
    dict_find(d, "ab");
    test_cond("typed dict set found", dict_found(d));
    dict_delete(d, "cd");
    dict_find(d, "cd");
    test_cond("typed dict set delete", !dict_found(d));
    test_cond("typed dict set size", dict_size(d) == 1);

    dict_free(d);
}

int main(void) {
    dict_typed_test_int_map();
    dict_typed_test_str_map();
    dict_typed_test_owned_map();
    dict_typed_test_set();
    test_report();
}
