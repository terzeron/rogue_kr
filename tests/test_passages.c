/*
 * Tests for passages.c numbering helpers
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <curses.h>
#include "rogue.h"

extern void passnum(void);

/* Test: passnum() leaves passages untouched when no exits exist */
static void test_passnum_no_exits(void **state) {
    (void) state;

    for (int i = 0; i < MAXROOMS; i++) {
        rooms[i].r_nexits = 0;
    }
    for (int i = 0; i < MAXPASS; i++) {
        passages[i].r_nexits = 0;
    }

    passnum();

    assert_int_equal(0, passages[0].r_nexits);
}

int run_passages_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_passnum_no_exits),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
