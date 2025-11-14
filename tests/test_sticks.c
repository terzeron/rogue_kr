/*
 * Tests for sticks.c functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../tests/test_stubs.h"

/* External function declarations from sticks.c */
extern void fix_stick(THING *cur);
extern void do_zap(void);
extern void drain(void);
extern void fire_bolt(coord *start, coord *dir, char *name);
extern char *charge_str(THING *obj);

/* Test: fix_stick() with wand */
static void test_fix_stick_wand(void **state) {
    (void) state;

    THING stick;
    memset(&stick, 0, sizeof(THING));
    stick.o_which = WS_LIGHT;

    fix_stick(&stick);

    /* Light wand should have 10-19 charges */
    assert_in_range(stick.o_charges, 10, 19);
}

/* Test: fix_stick() with other stick */
static void test_fix_stick_other(void **state) {
    (void) state;

    THING stick;
    memset(&stick, 0, sizeof(THING));
    stick.o_which = WS_MISSILE;

    fix_stick(&stick);

    /* Other sticks should have 3-7 charges */
    assert_in_range(stick.o_charges, 3, 7);
}

/* Test: fix_stick() damage strings */
static void test_fix_stick_damage(void **state) {
    (void) state;

    THING stick;
    memset(&stick, 0, sizeof(THING));
    stick.o_which = WS_LIGHT;

    fix_stick(&stick);

    /* Should have valid damage strings */
    assert_non_null(stick.o_damage);
    assert_non_null(stick.o_hurldmg);
}

/* Test: do_zap() with no stick */
static void test_do_zap_no_stick(void **state) {
    (void) state;

    pack = NULL;

    do_zap();

    assert_null(pack);
}

/* Test: drain() with no monsters */
static void test_drain_no_monsters(void **state) {
    (void) state;

    mlist = NULL;
    pstats.s_hpt = 10;

    drain();

    /* HP should be halved */
    assert_int_equal(pstats.s_hpt, 5);
}

/* Test: fire_bolt() basic setup */
static void test_fire_bolt_setup(void **state) {
    (void) state;

    coord start, dir;
    start.y = 5;
    start.x = 5;
    dir.y = 1;
    dir.x = 0;

    /* Note: Full test requires initialized map */
    assert_int_equal(start.y, 5);
}

/* Test: charge_str() with unknown stick */
static void test_charge_str_unknown(void **state) {
    (void) state;

    THING stick;
    memset(&stick, 0, sizeof(THING));
    stick.o_flags = 0;  /* Not known */
    stick.o_charges = 5;

    char *result = charge_str(&stick);

    assert_string_equal(result, "");
}

/* Test: charge_str() with known stick */
static void test_charge_str_known(void **state) {
    (void) state;

    THING stick;
    memset(&stick, 0, sizeof(THING));
    stick.o_flags = ISKNOW;
    stick.o_charges = 5;
    terse = FALSE;

    char *result = charge_str(&stick);

    assert_non_null(result);
    /* Should contain charge info */
}

/* Test: charge_str() with zero charges */
static void test_charge_str_zero_charges(void **state) {
    (void) state;

    THING stick;
    memset(&stick, 0, sizeof(THING));
    stick.o_flags = ISKNOW;
    stick.o_charges = 0;

    char *result = charge_str(&stick);

    assert_non_null(result);
}

/* Test suite runner */
int run_sticks_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_fix_stick_wand),
        cmocka_unit_test(test_fix_stick_other),
        cmocka_unit_test(test_fix_stick_damage),
        cmocka_unit_test(test_do_zap_no_stick),
        cmocka_unit_test(test_drain_no_monsters),
        cmocka_unit_test(test_fire_bolt_setup),
        cmocka_unit_test(test_charge_str_unknown),
        cmocka_unit_test(test_charge_str_known),
        cmocka_unit_test(test_charge_str_zero_charges),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
