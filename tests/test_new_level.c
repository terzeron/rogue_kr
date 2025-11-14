/*
 * Tests for new_level.c helper functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <curses.h>
#include "rogue.h"

extern int rnd_room(void);

/* Test: rnd_room() selects a non-gone room */
static void test_rnd_room_finds_valid_room(void **state) {
    (void) state;

    for (int i = 0; i < MAXROOMS; i++) {
        rooms[i].r_flags = ISGONE;
    }
    rooms[3].r_flags = 0;

    assert_int_equal(3, rnd_room());
}

int run_new_level_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_rnd_room_finds_valid_room),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
