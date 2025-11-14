/*
 * Tests for command.c functionality
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>
#include "rogue.h"
#include "i18n.h"

/* External declarations */
extern void init_helpstr(void);
extern bool levit_check(void);
extern void illcom(int ch);
extern void d_level(void);
extern void u_level(void);

/* Global state from extern.c */
extern struct h_list helpstr[];

static void set_hero_tile(char ch) {
    hero.y = 5;
    hero.x = 5;
    places[(hero.x << 5) + hero.y].p_ch = ch;
}

/* Test: init_helpstr() populates descriptions from catalog */
static void test_init_helpstr_populates(void **state) {
    (void) state;

    init_helpstr();
    assert_string_equal(helpstr[0].h_desc, msg_get("MSG_HELP_PRINTS_HELP"));
    assert_string_equal(helpstr[1].h_desc, msg_get("MSG_HELP_IDENTIFY_OBJECT"));
}

/* Test: levit_check() returns false when not levitating */
static void test_levit_check_false(void **state) {
    (void) state;

    player.t_flags &= ~ISLEVIT;
    assert_false(levit_check());
}

/* Test: levit_check() returns true when ISLEVIT set */
static void test_levit_check_true(void **state) {
    (void) state;

    player.t_flags |= ISLEVIT;
    assert_true(levit_check());
    player.t_flags &= ~ISLEVIT;
}

/* Test: illcom() clears count and preserves save_msg */
static void test_illcom_resets_state(void **state) {
    (void) state;

    count = 5;
    save_msg = TRUE;
    illcom('x');
    assert_int_equal(0, count);
    assert_true(save_msg);
}

/* Test: d_level() when no stairs leaves level unchanged */
static void test_d_level_without_stairs(void **state) {
    (void) state;

    level = 3;
    player.t_flags &= ~ISLEVIT;
    set_hero_tile(FLOOR);

    d_level();

    assert_int_equal(3, level);
}

/* Test: u_level() when no stairs leaves level unchanged */
static void test_u_level_without_stairs(void **state) {
    (void) state;

    level = 4;
    player.t_flags &= ~ISLEVIT;
    amulet = FALSE;
    set_hero_tile(FLOOR);

    u_level();

    assert_int_equal(4, level);
}

int run_command_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_init_helpstr_populates),
        cmocka_unit_test(test_levit_check_false),
        cmocka_unit_test(test_levit_check_true),
        cmocka_unit_test(test_illcom_resets_state),
        cmocka_unit_test(test_d_level_without_stairs),
        cmocka_unit_test(test_u_level_without_stairs),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
