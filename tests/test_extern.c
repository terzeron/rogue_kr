/*
 * Tests for extern.c global initialization
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdbool.h>
#include <curses.h>
#include "rogue.h"

/* Test: basic global flags have expected defaults */
static void test_global_flag_defaults(void **state) {
    (void) state;

    assert_true(playing);
    assert_false(amulet);
    assert_false(door_stop);
    assert_true(save_msg);
}

/* Test: pack_used entries start as FALSE */
static void test_pack_used_defaults(void **state) {
    (void) state;

    for (int i = 0; i < 26; i++) {
        assert_false(pack_used[i]);
    }
}

/* Test: monster table contains valid definitions */
static void test_monster_definitions(void **state) {
    (void) state;

    for (int i = 0; i < 5; i++) {
        assert_non_null(monsters[i].m_name);
        assert_true(monsters[i].m_stats.s_lvl >= 1);
        assert_true(monsters[i].m_stats.s_hpt >= 0);
    }
}

int run_extern_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_global_flag_defaults),
        cmocka_unit_test(test_pack_used_defaults),
        cmocka_unit_test(test_monster_definitions),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
