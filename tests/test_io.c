/*
 * Tests for io.c helper functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <curses.h>
#include "rogue.h"

/* Test: step_ok rejects walls */
static void test_step_ok_blocks_walls(void **state) {
    (void) state;

    assert_false(step_ok(' '));
    assert_false(step_ok('|'));
    assert_false(step_ok('-'));
}

/* Test: step_ok allows floor and monsters */
static void test_step_ok_allows_other_tiles(void **state) {
    (void) state;

    assert_true(step_ok('.'));
    assert_true(step_ok('*'));
}

int run_io_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_step_ok_blocks_walls),
        cmocka_unit_test(test_step_ok_allows_other_tiles),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
