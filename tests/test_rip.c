/*
 * Tests for rip.c helper functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <curses.h>
#include "rogue.h"
#include "i18n.h"

extern char *killname(char monst, bool doart);

/* Test: killname() maps starvation to translated string */
static void test_killname_starvation(void **state) {
    (void) state;

    const char *expected = msg_get("MSG_DEATH_STARVATION");
    assert_string_equal(expected, killname('s', FALSE));
}

/* Test: killname() returns monster name for uppercase code */
static void test_killname_monster(void **state) {
    (void) state;

    const char *name = killname('A', FALSE);
    assert_non_null(name);
    assert_true(strlen(name) > 0);
}

/* Test: killname() arrow cause */
static void test_killname_arrow(void **state) {
    (void) state;

    const char *expected = msg_get("MSG_DEATH_ARROW");
    assert_string_equal(expected, killname('a', FALSE));
}

int run_rip_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_killname_starvation),
        cmocka_unit_test(test_killname_monster),
        cmocka_unit_test(test_killname_arrow),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
