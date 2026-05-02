#include "list.h"
#include "testhelp.h"

#include <stdlib.h>
#include <string.h>

static int destroy_count = 0;
static int copy_count = 0;
static int copy_fail_after = -1;

static int strdup_copy(void *dst, const void *src) {
    const char *value = *(char *const *)src;
    char *copy = malloc(strlen(value) + 1);
    if (copy == NULL) return -1;
    memcpy(copy, value, strlen(value) + 1);
    *(char **)dst = copy;
    return 0;
}

static void tracked_str_ptr_destroy(void *elem) {
    destroy_count++;
    free(*(char **)elem);
}

static int int_copy_with_fail(void *dst, const void *src) {
    if (copy_fail_after >= 0 && copy_count >= copy_fail_after) {
        return -1;
    }

    copy_count++;
    memcpy(dst, src, sizeof(int));
    return 0;
}

static void list_test_int_runtime(void) {
    list *l = list_new(sizeof(int), NULL, NULL);
    list_node *middle;
    int expected[] = {0, 99, 88, 2};
    size_t i = 0;

    test_cond("list init", l != NULL);
    test_cond("list push front", list_push_front_value(l, int, 1) == 0);
    test_cond("list push front", list_push_front_value(l, int, 0) == 0);
    test_cond("list push back", list_push_back_value(l, int, 2) == 0);
    test_cond("list push back", list_push_back_value(l, int, 3) == 0);
    test_cond("list front", *list_front_as(l, int) == 0);
    test_cond("list back", *list_back_as(l, int) == 3);

    test_cond("list insert head", list_insert_value(l, 0, int, 7) == 0);
    test_cond("list insert middle", list_insert_value(l, 2, int, 4) == 0);
    middle = list_node_at(l, 3);
    test_cond("list node at", middle != NULL && *list_node_value_as(middle, int) == 1);
    test_cond("list insert before node", list_insert_before_value(l, middle, int, 99) == 0);
    test_cond("list insert after node", list_insert_after_value(l, middle, int, 88) == 0);
    test_cond("list set", list_set_value(l, 2, int, 42) == 0);
    test_cond("list set value", *list_get_as(l, 2, int) == 42);
    list_erase(l, 2);
    test_cond("list erase index", *list_get_as(l, 2, int) == 99);
    list_erase_node(l, middle);
    test_cond("list erase node", list_size(l) == 6);
    test_cond("list pop front", list_pop_front(l) == 0);
    test_cond("list pop back", list_pop_back(l) == 0);
    test_cond("list size", list_size(l) == 4);
    test_cond("list const access", *list_get_const_as(l, 1, int) == 99);

    for (list_node *node = list_front_node(l); node != NULL; node = node->next) {
        test_cond("list iterate order", *list_node_value_as(node, int) == expected[i]);
        i++;
    }
    test_cond("list iterate count", i == sizeof(expected) / sizeof(expected[0]));

    list_clear(l);
    test_cond("list clear", list_empty(l));
    test_cond("list front empty", list_front(l) == NULL);
    test_cond("list back empty", list_back(l) == NULL);
    test_cond("list pop front empty", list_pop_front(l) == -1);
    test_cond("list pop back empty", list_pop_back(l) == -1);

    list_free(l);
}

static void list_test_copy_clone(void) {
    list src;
    list dst;
    list *clone;

    destroy_count = 0;
    test_cond("list copy init src", list_init(&src, sizeof(char *), tracked_str_ptr_destroy, strdup_copy) == 0);
    test_cond("list copy init dst", list_init(&dst, sizeof(int), NULL, NULL) == 0);
    test_cond("list copy fill", list_push_back_value(&src, char *, "alpha") == 0);
    test_cond("list copy fill", list_push_front_value(&src, char *, "beta") == 0);

    test_cond("list copy null dst", list_copy(NULL, &src) == -1);
    test_cond("list copy null src", list_copy(&dst, NULL) == -1);
    test_cond("list copy success", list_copy(&dst, &src) == 0);
    test_cond("list copy contents",
              strcmp(*list_front_as(&dst, char *), "beta") == 0 &&
              strcmp(*list_back_as(&dst, char *), "alpha") == 0);

    clone = list_clone(&src);
    test_cond("list clone alloc", clone != NULL);
    test_cond("list clone contents", strcmp(*list_front_as(clone, char *), "beta") == 0);
    test_cond("list clone update", list_set_value(clone, 1, char *, "gamma") == 0);
    test_cond("list clone independent",
              strcmp(*list_back_as(clone, char *), "gamma") == 0 &&
              strcmp(*list_back_as(&src, char *), "alpha") == 0);

    list_free(clone);
    list_destroy(&dst);
    list_destroy(&src);
}

static void list_test_ptr_runtime(void) {
    list *l = list_new(sizeof(char *), tracked_str_ptr_destroy, strdup_copy);
    list_node *node;

    destroy_count = 0;
    test_cond("list ptr init", l != NULL);
    test_cond("list ptr push back", list_push_back_value(l, char *, "beta") == 0);
    test_cond("list ptr push front", list_push_front_value(l, char *, "alpha") == 0);
    test_cond("list ptr push back", list_push_back_value(l, char *, "gamma") == 0);
    node = list_node_at(l, 1);
    test_cond("list ptr insert before", list_insert_before_value(l, node, char *, "between") == 0);
    test_cond("list ptr front", strcmp(*list_front_as(l, char *), "alpha") == 0);
    test_cond("list ptr node value", strcmp(*list_node_value_as(node, char *), "beta") == 0);
    test_cond("list ptr set", list_set_node_value(l, node, char *, "delta") == 0);
    test_cond("list ptr set destroy", destroy_count == 1);
    test_cond("list ptr set copy", strcmp(*list_node_value_as(node, char *), "delta") == 0);
    list_erase(l, 1);
    test_cond("list ptr erase destroy", destroy_count == 2);
    test_cond("list ptr pop back", list_pop_back(l) == 0);
    test_cond("list ptr pop destroy", destroy_count == 3);
    list_clear(l);
    test_cond("list ptr clear", list_empty(l));
    test_cond("list ptr clear destroy", destroy_count == 5);

    list_free(l);
}

static void list_test_runtime_failures(void) {
    list l;

    test_cond("list init invalid", list_init(NULL, sizeof(int), NULL, NULL) == -1);
    test_cond("list init invalid elem", list_init(&l, 0, NULL, NULL) == -1);
    test_cond("list init stack", list_init(&l, sizeof(int), NULL, NULL) == 0);
    test_cond("list push null elem", list_push_back(&l, NULL) == -1);
    test_cond("list insert out of range", list_insert(&l, 1, &(int){1}) == -1);
    test_cond("list insert before null", list_insert_before(&l, NULL, &(int){1}) == -1);
    test_cond("list insert after null", list_insert_after(&l, NULL, &(int){1}) == -1);
    test_cond("list set out of range", list_set(&l, 0, &(int){1}) == -1);
    test_cond("list get out of range", list_at(&l, 0) == NULL);
    list_destroy(&l);

    list *fail_list = list_new(sizeof(int), NULL, int_copy_with_fail);
    copy_count = 0;
    copy_fail_after = -1;
    test_cond("list init fail copy", fail_list != NULL);
    test_cond("list copy success", list_push_back_value(fail_list, int, 11) == 0);
    copy_fail_after = 1;
    test_cond("list copy failure push back", list_push_back_value(fail_list, int, 22) == -1);
    test_cond("list copy failure push front", list_push_front_value(fail_list, int, 33) == -1);
    test_cond("list copy failure insert", list_insert_value(fail_list, 0, int, 44) == -1);
    test_cond("list copy failure set", list_set_value(fail_list, 0, int, 55) == -1);
    test_cond("list copy failure size", list_size(fail_list) == 1);
    test_cond("list copy preserve", *list_front_as(fail_list, int) == 11);
    copy_fail_after = -1;
    list_free(fail_list);
}

int main(void) {
    list_test_int_runtime();
    list_test_copy_clone();
    list_test_ptr_runtime();
    list_test_runtime_failures();
    test_report();
}
