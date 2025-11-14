/*
 * Tests for wizard.c helper utilities
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "rogue.h"

extern const char *type_name(int type);
extern void set_know(THING *obj, struct obj_info *info);

/* Test: type_name returns readable descriptions */
static void test_type_name_weapon(void **state) {
    (void) state;
    assert_string_equal("weapon", type_name(WEAPON));
}

/* Test: set_know marks object info as known */
static void test_set_know_marks_known(void **state) {
    (void) state;

    struct obj_info info[1] = {
        {"test", 0, 0, NULL, FALSE},
    };
    info[0].oi_guess = strdup("mystery");
    THING obj;
    memset(&obj, 0, sizeof(obj));
    obj.o_which = 0;

    set_know(&obj, info);

    assert_true(info[0].oi_know);
    assert_true(obj.o_flags & ISKNOW);
    assert_null(info[0].oi_guess);
}

int run_wizard_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_type_name_weapon),
        cmocka_unit_test(test_set_know_marks_known),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
