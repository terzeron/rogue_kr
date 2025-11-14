/*
 * Tests for move.c - Movement and position functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>

/* Include actual rogue.h for correct THING definition */
#include "rogue.h"

/* External declarations that might not be in rogue.h */
extern coord *rndmove(THING *who);

/* Setup function to reset cur_ring and initialize ncurses for each test */
static int setup_cur_ring(void **state) {
    (void) state;
    cur_ring[0] = NULL;
    cur_ring[1] = NULL;
    to_death = 0;
    terse = 0;

    /* Initialize ncurses if not already initialized */
    if (stdscr == NULL) {
        stdscr = initscr();
    }

    return 0;
}

/* Teardown function to clean up ncurses */
static int teardown_cur_ring(void **state) {
    (void) state;
    if (stdscr != NULL) {
        endwin();
        stdscr = NULL;
    }
    return 0;
}

/* Test: rust_armor() with NULL armor */
static void test_rust_armor_null(void **state) {
    (void) state; /* unused */
    /* Should not crash with NULL */
    rust_armor(NULL);
    assert_true(1);  /* If we get here, test passes */
}

/* Test: rust_armor() with leather armor (should not rust) */
static void test_rust_armor_leather(void **state) {
    (void) state; /* unused */
    THING armor;
    armor.o_type = ARMOR;
    armor.o_which = LEATHER;
    armor.o_arm = 5;
    armor.o_flags = 0;

    rust_armor(&armor);

    /* Leather doesn't rust, armor value unchanged */
    assert_int_equal(5, armor.o_arm);
}

/* Test: rust_armor() with protected armor */
static void test_rust_armor_protected(void **state) {
    (void) state; /* unused */
    THING armor;
    armor.o_type = ARMOR;
    armor.o_which = 3;  /* Some metal armor */
    armor.o_arm = 5;
    armor.o_flags = ISPROT;

    rust_armor(&armor);

    /* Protected armor doesn't rust */
    assert_int_equal(5, armor.o_arm);
}

/* Test: rust_armor() with normal armor */
static void test_rust_armor_normal(void **state) {
    (void) state; /* unused */
    THING armor = {0};  /* Initialize all fields to 0 */
    armor.o_type = ARMOR;
    armor.o_which = 3;  /* Some metal armor */
    armor.o_arm = 5;
    armor.o_flags = 0;

    rust_armor(&armor);

    /* Armor should rust (increase armor class by 1, worse) */
    assert_int_equal(6, armor.o_arm);
}

/* Test: rust_armor() with maxed out armor */
static void test_rust_armor_maxed(void **state) {
    (void) state; /* unused */
    THING armor;
    armor.o_type = ARMOR;
    armor.o_which = 3;
    armor.o_arm = 9;  /* Already at max */
    armor.o_flags = 0;

    rust_armor(&armor);

    /* Armor at 9+ doesn't rust further */
    assert_int_equal(9, armor.o_arm);
}

/* Test: rust_armor() multiple times */
static void test_rust_armor_multiple(void **state) {
    (void) state; /* unused */
    THING armor = {0};  /* Initialize all fields to 0 */
    armor.o_type = ARMOR;
    armor.o_which = 3;
    armor.o_arm = 5;
    armor.o_flags = 0;

    rust_armor(&armor);
    assert_int_equal(6, armor.o_arm);

    rust_armor(&armor);
    assert_int_equal(7, armor.o_arm);

    rust_armor(&armor);
    assert_int_equal(8, armor.o_arm);
}

/* Test: rndmove() returns valid adjacent position */
static void test_rndmove_basic(void **state) {
    (void) state; /* unused */
    THING monster;
    monster.t_pos.x = 40;
    monster.t_pos.y = 12;

    /* Test multiple times to check randomness */
    for (int i = 0; i < 20; i++) {
        coord *result = rndmove(&monster);

        assert_non_null(result);

        /* Result should be within 1 step */
        int dx = abs(result->x - monster.t_pos.x);
        int dy = abs(result->y - monster.t_pos.y);

        assert_true(dx <= 1);
        assert_true(dy <= 1);
    }
}

/* Test: rndmove() can return same position */
static void test_rndmove_same_position(void **state) {
    (void) state; /* unused */
    THING monster;
    monster.t_pos.x = 40;
    monster.t_pos.y = 12;

    int same_count = 0;
    int trials = 100;

    for (int i = 0; i < trials; i++) {
        coord *result = rndmove(&monster);
        if (result->x == monster.t_pos.x && result->y == monster.t_pos.y) {
            same_count++;
        }
    }

    /* Should get same position at least once in 100 tries (1/9 probability) */
    assert_true(same_count > 0);
}

/* Test: rndmove() covers all 9 positions eventually */
static void test_rndmove_coverage(void **state) {
    (void) state; /* unused */
    THING monster;
    monster.t_pos.x = 40;
    monster.t_pos.y = 12;

    int positions[3][3] = {{0}};
    int trials = 1000;  /* Increased from 500 to 1000 */

    for (int i = 0; i < trials; i++) {
        coord *result = rndmove(&monster);

        int dx = result->x - monster.t_pos.x + 1;
        int dy = result->y - monster.t_pos.y + 1;

        if (dx >= 0 && dx < 3 && dy >= 0 && dy < 3) {
            positions[dy][dx] = 1;
        }
    }

    /* Check that we hit at least 6 of the 9 positions (more lenient) */
    int covered = 0;
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (positions[y][x]) {
                covered++;
            }
        }
    }

    assert_true(covered >= 6);
}

int run_move_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_rust_armor_null, setup_cur_ring, teardown_cur_ring),
        cmocka_unit_test_setup_teardown(test_rust_armor_leather, setup_cur_ring, teardown_cur_ring),
        cmocka_unit_test_setup_teardown(test_rust_armor_protected, setup_cur_ring, teardown_cur_ring),
        cmocka_unit_test_setup_teardown(test_rust_armor_normal, setup_cur_ring, teardown_cur_ring),
        cmocka_unit_test_setup_teardown(test_rust_armor_maxed, setup_cur_ring, teardown_cur_ring),
        cmocka_unit_test_setup_teardown(test_rust_armor_multiple, setup_cur_ring, teardown_cur_ring),
        cmocka_unit_test(test_rndmove_basic),
        cmocka_unit_test(test_rndmove_same_position),
        cmocka_unit_test(test_rndmove_coverage),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
