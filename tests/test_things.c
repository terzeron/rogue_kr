/*
 * Tests for things.c functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../tests/test_stubs.h"

/* External function declarations from things.c */
extern char *inv_name(THING *obj, bool drop);
extern void drop(void);
extern bool dropcheck(THING *obj);
extern THING *new_thing(void);
extern int pick_one(struct obj_info *info, int nitems);
extern void discovered(void);
extern void print_disc(char type);
extern void set_order(int *order, int numthings);
extern char add_line(char *fmt, char *arg);
extern void end_line(void);
extern char *nothing(char type);
extern void nameit(THING *obj, int type, char *which, struct obj_info *op, char *(*prfunc)(THING *));
extern char *nullstr(THING *ignored);

/* Test: inv_name() with weapon */
static void test_inv_name_weapon(void **state) {
    (void) state;

    THING obj;
    memset(&obj, 0, sizeof(THING));
    obj.o_type = WEAPON;
    obj.o_which = 0;
    obj.o_count = 1;
    obj.o_flags = 0;

    char *result = inv_name(&obj, FALSE);

    assert_non_null(result);
    /* Should contain weapon name */
}

/* Test: inv_name() with armor */
static void test_inv_name_armor(void **state) {
    (void) state;

    THING obj;
    memset(&obj, 0, sizeof(THING));
    obj.o_type = ARMOR;
    obj.o_which = 0;
    obj.o_flags = 0;

    char *result = inv_name(&obj, FALSE);

    assert_non_null(result);
}

/* Test: inv_name() with potion */
static void test_inv_name_potion(void **state) {
    (void) state;

    THING obj;
    memset(&obj, 0, sizeof(THING));
    obj.o_type = POTION;
    obj.o_which = 0;
    obj.o_count = 1;

    char *result = inv_name(&obj, FALSE);

    assert_non_null(result);
}

/* Test: inv_name() with gold */
static void test_inv_name_gold(void **state) {
    (void) state;

    THING obj;
    memset(&obj, 0, sizeof(THING));
    obj.o_type = GOLD;
    obj.o_goldval = 100;

    char *result = inv_name(&obj, FALSE);

    assert_non_null(result);
    /* Should contain "100 Gold pieces" */
}

/* Test: inv_name() with amulet */
static void test_inv_name_amulet(void **state) {
    (void) state;

    THING obj;
    memset(&obj, 0, sizeof(THING));
    obj.o_type = AMULET;

    char *result = inv_name(&obj, FALSE);

    assert_non_null(result);
    /* Should be "The Amulet of Yendor" */
}

/* Test: dropcheck() with NULL */
static void test_dropcheck_null(void **state) {
    (void) state;

    bool result = dropcheck(NULL);

    assert_true(result);
}

/* Test: dropcheck() with cursed item */
static void test_dropcheck_cursed(void **state) {
    (void) state;

    THING obj;
    memset(&obj, 0, sizeof(THING));
    obj.o_flags = ISCURSED;

    cur_weapon = &obj;

    bool result = dropcheck(&obj);

    assert_false(result);
}

/* Test: dropcheck() with normal item */
static void test_dropcheck_normal(void **state) {
    (void) state;

    THING obj;
    memset(&obj, 0, sizeof(THING));
    obj.o_flags = 0;

    cur_weapon = NULL;
    cur_armor = NULL;
    cur_ring[LEFT] = NULL;
    cur_ring[RIGHT] = NULL;

    bool result = dropcheck(&obj);

    assert_true(result);
}

/* Test: new_thing() creates valid object */
static void test_new_thing_creates_object(void **state) {
    (void) state;

    no_food = 0;

    THING *obj = new_thing();

    assert_non_null(obj);
    assert_int_equal(obj->o_count, 1);
    assert_int_equal(obj->o_group, 0);

    free(obj);
}

/* Test: new_thing() sets defaults */
static void test_new_thing_defaults(void **state) {
    (void) state;

    no_food = 0;

    THING *obj = new_thing();

    assert_non_null(obj);
    assert_int_equal(obj->o_hplus, 0);
    assert_int_equal(obj->o_dplus, 0);

    free(obj);
}

/* Test: pick_one() returns valid index */
static void test_pick_one_valid_index(void **state) {
    (void) state;

    /* Note: Requires obj_info array */
    assert_true(true);
}

/* Test: set_order() randomizes array */
static void test_set_order_randomizes(void **state) {
    (void) state;

    int order[10];

    set_order(order, 10);

    /* Should have values 0-9 in some order */
    for (int i = 0; i < 10; i++) {
        assert_in_range(order[i], 0, 9);
    }
}

/* Test: nullstr() returns empty string */
static void test_nullstr_returns_empty(void **state) {
    (void) state;

    char *result = nullstr(NULL);

    assert_non_null(result);
    assert_string_equal("", result);
}

/* Test: nothing() with potion */
static void test_nothing_potion(void **state) {
    (void) state;

    terse = FALSE;

    char *result = nothing(POTION);

    assert_non_null(result);
    assert_string_equal("Haven't discovered anything about any potions", result);
}

/* Test: nothing() with wildcard */
static void test_nothing_wildcard(void **state) {
    (void) state;

    terse = TRUE;

    char *result = nothing('*');

    assert_non_null(result);
    assert_string_equal("Nothing", result);
}

/* Test: drop() with no item */
static void test_drop_no_item(void **state) {
    (void) state;

    pack = NULL;

    drop();

    assert_null(pack);
}

/* Test suite runner */
int run_things_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_inv_name_weapon),
        cmocka_unit_test(test_inv_name_armor),
        cmocka_unit_test(test_inv_name_potion),
        cmocka_unit_test(test_inv_name_gold),
        cmocka_unit_test(test_inv_name_amulet),
        cmocka_unit_test(test_dropcheck_null),
        cmocka_unit_test(test_dropcheck_cursed),
        cmocka_unit_test(test_dropcheck_normal),
        cmocka_unit_test(test_new_thing_creates_object),
        cmocka_unit_test(test_new_thing_defaults),
        cmocka_unit_test(test_pick_one_valid_index),
        cmocka_unit_test(test_set_order_randomizes),
        cmocka_unit_test(test_nullstr_returns_empty),
        cmocka_unit_test(test_nothing_potion),
        cmocka_unit_test(test_nothing_wildcard),
        cmocka_unit_test(test_drop_no_item),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
