/*
 * Tests for daemons.c functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>
#include "rogue.h"

extern void doctor(void);
extern void stomach(void);
extern void nohaste(void);
extern void unconfuse(void);
extern void swander(void);
extern void land(void);
extern void rollwand(void);

extern int quiet;
extern int food_left;
extern int hungry_state;
extern bool terse;
extern bool amulet;

static void clear_daemon_slots(void) {
    extern struct delayed_action d_list[];
    for (int i = 0; i < MAXDAEMONS; i++) {
        d_list[i].d_type = 0;
        d_list[i].d_func = NULL;
        d_list[i].d_arg = 0;
        d_list[i].d_time = 0;
    }
}

/* Test: doctor() heals low level players after enough quiet time */
static void test_doctor_heals_low_level(void **state) {
    (void) state;

    pstats.s_lvl = 5;
    pstats.s_hpt = 10;
    player.t_stats.s_maxhp = 15;
    quiet = 20;
    cur_ring[LEFT] = NULL;
    cur_ring[RIGHT] = NULL;

    doctor();

    assert_true(pstats.s_hpt >= 11);
    assert_int_equal(0, quiet);
}

/* Test: stomach() consumes food and updates hunger state */
static void test_stomach_consumes_food(void **state) {
    (void) state;

    food_left = MORETIME + 10;
    hungry_state = 0;
    amulet = FALSE;
    terse = TRUE;
    cur_ring[LEFT] = NULL;
    cur_ring[RIGHT] = NULL;

    stomach();

    assert_int_equal(MORETIME + 9, food_left);
    assert_int_equal(0, hungry_state);
}

/* Test: nohaste() clears ISHASTE flag */
static void test_nohaste_clears_flag(void **state) {
    (void) state;

    player.t_flags |= ISHASTE;
    nohaste();
    assert_false(on(player, ISHASTE));
}

/* Test: unconfuse() clears ISHUH flag */
static void test_unconfuse_clears_flag(void **state) {
    (void) state;

    player.t_flags |= ISHUH;
    unconfuse();
    assert_false(on(player, ISHUH));
}

/* Test: swander() schedules rollwand daemon */
static void test_swander_registers_rollwand(void **state) {
    (void) state;

    clear_daemon_slots();
    swander();

    int found = 0;
    for (int i = 0; i < MAXDAEMONS; i++) {
        if (d_list[i].d_func == rollwand) {
            found = 1;
            assert_true(d_list[i].d_type != 0);
            assert_true(d_list[i].d_time <= 0);
            break;
        }
    }
    assert_true(found);
}

/* Test: land() removes levitation flag */
static void test_land_clears_levitation(void **state) {
    (void) state;

    player.t_flags |= ISLEVIT;
    land();
    assert_false(on(player, ISLEVIT));
}

int run_daemons_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_doctor_heals_low_level),
        cmocka_unit_test(test_stomach_consumes_food),
        cmocka_unit_test(test_nohaste_clears_flag),
        cmocka_unit_test(test_unconfuse_clears_flag),
        cmocka_unit_test(test_swander_registers_rollwand),
        cmocka_unit_test(test_land_clears_levitation),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
