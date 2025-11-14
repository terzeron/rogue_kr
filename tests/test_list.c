/*
 * Tests for list.c - Linked list management functions using CMocka
 */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>

/* Type definitions from rogue.h */
typedef struct {
    int x;
    int y;
} coord;

typedef union thing {
    struct {
        union thing *_l_next, *_l_prev;
        coord _t_pos;
        char _t_type;
        int _t_flags;
    } _t;
} THING;

#define l_next _t._l_next
#define l_prev _t._l_prev
#define next(ptr) (ptr)->l_next
#define prev(ptr) (ptr)->l_prev

/* Function declarations from list.c */
extern void _attach(THING **list, THING *item);
extern void _detach(THING **list, THING *item);
extern void _free_list(THING **list);
extern THING *new_item();
extern void discard(THING *item);

/* Helper to create a simple test item */
static THING *create_test_item() {
    THING *item = (THING *)calloc(1, sizeof(THING));
    assert_non_null(item);
    item->l_next = NULL;
    item->l_prev = NULL;
    return item;
}

static void test_attach_single_item(void **state) {
    (void) state; /* unused */
    THING *list = NULL;
    THING *item = create_test_item();

    _attach(&list, item);

    assert_non_null(list);
    assert_ptr_equal(list, item);
    assert_null(item->l_next);
    assert_null(item->l_prev);

    free(item);
}

static void test_attach_multiple_items(void **state) {
    (void) state; /* unused */
    THING *list = NULL;
    THING *item1 = create_test_item();
    THING *item2 = create_test_item();
    THING *item3 = create_test_item();

    _attach(&list, item1);
    _attach(&list, item2);
    _attach(&list, item3);

    /* List should be: item3 -> item2 -> item1 */
    assert_ptr_equal(list, item3);
    assert_ptr_equal(item3->l_next, item2);
    assert_ptr_equal(item2->l_next, item1);
    assert_null(item1->l_next);

    assert_null(item3->l_prev);
    assert_ptr_equal(item2->l_prev, item3);
    assert_ptr_equal(item1->l_prev, item2);

    free(item1);
    free(item2);
    free(item3);
}

static void test_detach_head(void **state) {
    (void) state; /* unused */
    THING *list = NULL;
    THING *item1 = create_test_item();
    THING *item2 = create_test_item();

    _attach(&list, item1);
    _attach(&list, item2);

    /* List is: item2 -> item1 */
    _detach(&list, item2);

    /* Now list should be: item1 */
    assert_ptr_equal(list, item1);
    assert_null(item1->l_next);
    assert_null(item1->l_prev);
    assert_null(item2->l_next);
    assert_null(item2->l_prev);

    free(item1);
    free(item2);
}

static void test_detach_middle(void **state) {
    (void) state; /* unused */
    THING *list = NULL;
    THING *item1 = create_test_item();
    THING *item2 = create_test_item();
    THING *item3 = create_test_item();

    _attach(&list, item1);
    _attach(&list, item2);
    _attach(&list, item3);

    /* List is: item3 -> item2 -> item1 */
    _detach(&list, item2);

    /* Now list should be: item3 -> item1 */
    assert_ptr_equal(list, item3);
    assert_ptr_equal(item3->l_next, item1);
    assert_ptr_equal(item1->l_prev, item3);
    assert_null(item2->l_next);
    assert_null(item2->l_prev);

    free(item1);
    free(item2);
    free(item3);
}

static void test_detach_tail(void **state) {
    (void) state; /* unused */
    THING *list = NULL;
    THING *item1 = create_test_item();
    THING *item2 = create_test_item();

    _attach(&list, item1);
    _attach(&list, item2);

    /* List is: item2 -> item1 */
    _detach(&list, item1);

    /* Now list should be: item2 */
    assert_ptr_equal(list, item2);
    assert_null(item2->l_next);
    assert_null(item2->l_prev);
    assert_null(item1->l_next);
    assert_null(item1->l_prev);

    free(item1);
    free(item2);
}

static void test_detach_only_item(void **state) {
    (void) state; /* unused */
    THING *list = NULL;
    THING *item = create_test_item();

    _attach(&list, item);
    _detach(&list, item);

    assert_null(list);
    assert_null(item->l_next);
    assert_null(item->l_prev);

    free(item);
}

static void test_new_item_creation(void **state) {
    (void) state; /* unused */
    THING *item = new_item();

    assert_non_null(item);
    assert_null(item->l_next);
    assert_null(item->l_prev);

    discard(item);
}

static void test_list_traversal(void **state) {
    (void) state; /* unused */
    THING *list = NULL;
    THING *items[5];
    int count;

    /* Build list of 5 items */
    for (int i = 0; i < 5; i++) {
        items[i] = create_test_item();
        _attach(&list, items[i]);
    }

    /* Count items by traversing */
    count = 0;
    for (THING *curr = list; curr != NULL; curr = curr->l_next) {
        count++;
    }

    assert_int_equal(5, count);

    /* Cleanup */
    for (int i = 0; i < 5; i++) {
        free(items[i]);
    }
}

/* Test: _free_list() with multiple items */
static void test_free_list_multiple(void **state) {
    (void) state; /* unused */
    THING *list = NULL;

    /* Create list with 3 items */
    for (int i = 0; i < 3; i++) {
        THING *item = create_test_item();
        _attach(&list, item);
    }

    assert_non_null(list);

    /* Free entire list */
    _free_list(&list);

    /* List should now be NULL */
    assert_null(list);
}

/* Test: _free_list() with empty list */
static void test_free_list_empty(void **state) {
    (void) state; /* unused */
    THING *list = NULL;

    /* Free empty list - should not crash */
    _free_list(&list);

    assert_null(list);
}

/* Test: _free_list() with single item */
static void test_free_list_single(void **state) {
    (void) state; /* unused */
    THING *list = NULL;
    THING *item = create_test_item();

    _attach(&list, item);
    assert_non_null(list);

    _free_list(&list);

    assert_null(list);
}

/* Test: discard() properly frees item */
static void test_discard_item(void **state) {
    (void) state; /* unused */
    THING *item = new_item();

    assert_non_null(item);

    /* discard() should free the item without crashing */
    discard(item);

    /* If we get here, test passes */
    assert_true(1);
}

int run_list_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_attach_single_item),
        cmocka_unit_test(test_attach_multiple_items),
        cmocka_unit_test(test_detach_head),
        cmocka_unit_test(test_detach_middle),
        cmocka_unit_test(test_detach_tail),
        cmocka_unit_test(test_detach_only_item),
        cmocka_unit_test(test_new_item_creation),
        cmocka_unit_test(test_list_traversal),
        cmocka_unit_test(test_free_list_multiple),
        cmocka_unit_test(test_free_list_empty),
        cmocka_unit_test(test_free_list_single),
        cmocka_unit_test(test_discard_item),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
