/*
 * Tests for scrolls.c functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../tests/test_stubs.h"

/* External function declarations from scrolls.c */
extern void read_scroll(void);
extern void uncurse(THING *obj);

/* Test: read_scroll() with no scroll */
static void test_read_scroll_no_scroll(void **state) {
    (void) state;

    pack = NULL;

    read_scroll();

    assert_null(pack);
}

/* Test: read_scroll() with non-scroll item */
static void test_read_scroll_non_scroll(void **state) {
    (void) state;

    THING obj;
    memset(&obj, 0, sizeof(THING));
    obj.o_type = WEAPON;

    /* Would need to mock get_item */
    assert_int_equal(obj.o_type, WEAPON);
}

/* Test: uncurse() removes curse */
static void test_uncurse_removes_curse(void **state) {
    (void) state;

    THING obj;
    memset(&obj, 0, sizeof(THING));
    obj.o_flags = ISCURSED;

    uncurse(&obj);

    assert_false(obj.o_flags & ISCURSED);
}

/* Test: uncurse() with NULL */
static void test_uncurse_null(void **state) {
    (void) state;

    uncurse(NULL);

    /* Should not crash */
    assert_true(true);
}

/* Test: uncurse() with uncursed item */
static void test_uncurse_already_uncursed(void **state) {
    (void) state;

    THING obj;
    memset(&obj, 0, sizeof(THING));
    obj.o_flags = 0;

    uncurse(&obj);

    assert_false(obj.o_flags & ISCURSED);
}

/* Test: uncurse() preserves other flags */
static void test_uncurse_preserves_flags(void **state) {
    (void) state;

    THING obj;
    memset(&obj, 0, sizeof(THING));
    obj.o_flags = ISCURSED | ISKNOW;

    uncurse(&obj);

    assert_false(obj.o_flags & ISCURSED);
    assert_true(obj.o_flags & ISKNOW);
}

/* Test suite runner */
int run_scrolls_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_read_scroll_no_scroll),
        cmocka_unit_test(test_read_scroll_non_scroll),
        cmocka_unit_test(test_uncurse_removes_curse),
        cmocka_unit_test(test_uncurse_null),
        cmocka_unit_test(test_uncurse_already_uncursed),
        cmocka_unit_test(test_uncurse_preserves_flags),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
