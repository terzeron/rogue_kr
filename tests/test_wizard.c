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
    const char *result = type_name(WEAPON);
    /* type_name now returns translated message, check it's not NULL */
    assert_non_null(result);
    /* In test environment, msg_get returns the key itself */
    /* So we check if it contains "WEAPON" or is the translated "weapon" */
    assert_true(strstr(result, "WEAPON") != NULL || strcmp(result, "weapon") == 0);
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
