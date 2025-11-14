/*
 * Tests for monsters.c functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>
#include "rogue.h"

/* External declarations */
extern char randmonster(bool wander);
extern int exp_add(THING *tp);

/* Test: randmonster() returns valid character */
static void test_randmonster_valid(void **state) {
    (void) state; /* unused */
    for (int i = 0; i < 50; i++) {
        char ch = randmonster(false);
        /* Should be uppercase letter A-Z */
        assert_true(ch >= 'A' && ch <= 'Z');
    }
}

/* Test: randmonster() variety */
static void test_randmonster_variety(void **state) {
    (void) state; /* unused */
    char monsters_chars[50];
    for (int i = 0; i < 50; i++) {
        monsters_chars[i] = randmonster(false);
    }

    /* Should have some variety */
    int all_same = 1;
    for (int i = 1; i < 50; i++) {
        if (monsters_chars[i] != monsters_chars[0]) {
            all_same = 0;
            break;
        }
    }
    assert_false(all_same);
}

/* Test: monster data integrity */
static void test_monster_data_valid(void **state) {
    (void) state; /* unused */
    /* Check first few monsters have valid data */
    for (int i = 0; i < 5; i++) {
        assert_non_null(monsters[i].m_name);
        assert_true(monsters[i].m_stats.s_lvl >= 1 && monsters[i].m_stats.s_lvl <= 30);
        assert_true(monsters[i].m_stats.s_str >= 1 && monsters[i].m_stats.s_str <= 100);
    }
}

/* Test: exp_add() for level 1 monster */
static void test_exp_add_level1(void **state) {
    (void) state; /* unused */
    THING monster = {0};
    monster.t_stats.s_lvl = 1;
    monster.t_stats.s_maxhp = 8;

    int exp = exp_add(&monster);
    /* Level 1: maxhp / 8 = 8/8 = 1 */
    assert_int_equal(1, exp);
}

/* Test: exp_add() for mid-level monster */
static void test_exp_add_mid_level(void **state) {
    (void) state; /* unused */
    THING monster = {0};
    monster.t_stats.s_lvl = 5;
    monster.t_stats.s_maxhp = 30;

    int exp = exp_add(&monster);
    /* Level 2-6: maxhp / 6 = 30/6 = 5 */
    assert_int_equal(5, exp);
}

/* Test: exp_add() for high-level monster (7-9) */
static void test_exp_add_high_level(void **state) {
    (void) state; /* unused */
    THING monster = {0};
    monster.t_stats.s_lvl = 8;
    monster.t_stats.s_maxhp = 48;

    int exp = exp_add(&monster);
    /* Level 7-9: (maxhp / 6) * 4 = (48/6) * 4 = 8 * 4 = 32 */
    assert_int_equal(32, exp);
}

/* Test: exp_add() for very high-level monster (10+) */
static void test_exp_add_very_high_level(void **state) {
    (void) state; /* unused */
    THING monster = {0};
    monster.t_stats.s_lvl = 15;
    monster.t_stats.s_maxhp = 90;

    int exp = exp_add(&monster);
    /* Level 10+: (maxhp / 6) * 20 = (90/6) * 20 = 15 * 20 = 300 */
    assert_int_equal(300, exp);
}

/* Test: save_throw() statistical test */
static void test_save_throw_probability(void **state) {
    (void) state; /* unused */
    THING creature = {0};
    creature.t_stats.s_lvl = 10;

    int successes = 0;
    int trials = 100;

    for (int i = 0; i < trials; i++) {
        if (save_throw(0, &creature)) {
            successes++;
        }
    }

    /* Level 10, which=0: need = 14 + 0 - 10/2 = 9
     * Success if roll(1,20) >= 9, which is 12/20 = 60%
     * In 100 trials, expect around 60 successes, allow 30-90 range */
    assert_true(successes >= 30 && successes <= 90);
}

/* Test: save_throw() with high which value (harder save) */
static void test_save_throw_hard(void **state) {
    (void) state; /* unused */
    THING creature = {0};
    creature.t_stats.s_lvl = 5;

    int successes = 0;
    int trials = 100;

    for (int i = 0; i < trials; i++) {
        if (save_throw(5, &creature)) {
            successes++;
        }
    }

    /* Level 5, which=5: need = 14 + 5 - 5/2 = 17 (rounded)
     * Success if roll(1,20) >= 17, which is 4/20 = 20%
     * In 100 trials, expect around 20 successes, allow 5-40 range */
    assert_true(successes >= 5 && successes <= 40);
}

int run_monsters_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_randmonster_valid),
        cmocka_unit_test(test_randmonster_variety),
        cmocka_unit_test(test_monster_data_valid),
        cmocka_unit_test(test_exp_add_level1),
        cmocka_unit_test(test_exp_add_mid_level),
        cmocka_unit_test(test_exp_add_high_level),
        cmocka_unit_test(test_exp_add_very_high_level),
        cmocka_unit_test(test_save_throw_probability),
        cmocka_unit_test(test_save_throw_hard),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
