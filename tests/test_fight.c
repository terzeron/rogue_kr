/*
 * Tests for fight.c functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>

/* Minimal type definitions */
typedef struct {
    unsigned int s_str;
    int s_exp;
    int s_lvl;
    int s_arm;
    int s_hpt;
    char s_dmg[13];
    int s_maxhp;
} stats_t;

typedef union {
    struct {
        void *l_next, *l_prev;
        int t_pos_y, t_pos_x;
        char t_type;
        stats_t t_stats;
    } _t;
} THING;

/* Function declarations from fight.c */
extern int swing(int at_lvl, int op_arm, int wplus);
extern int roll(int number, int sides);

/* Test: swing() basic probability */
static void test_swing_basic(void **state) {
    (void) state; /* unused */
    int hits = 0;
    int trials = 100;

    /* Level 5 vs armor 10 with +0 weapon */
    for (int i = 0; i < trials; i++) {
        if (swing(5, 10, 0)) {
            hits++;
        }
    }

    /* Should hit some but not all */
    assert_true(hits >= 20 && hits <= 80);
}

/* Test: swing() with bonuses */
static void test_swing_with_bonus(void **state) {
    (void) state; /* unused */
    int hits_normal = 0;
    int hits_bonus = 0;
    int trials = 100;

    for (int i = 0; i < trials; i++) {
        if (swing(5, 10, 0)) hits_normal++;
        if (swing(5, 10, 5)) hits_bonus++;
    }

    /* Bonus weapon should hit more often */
    assert_true(hits_bonus >= hits_normal);
}

/* Test: swing() high level vs low armor */
static void test_swing_easy_hit(void **state) {
    (void) state; /* unused */
    int hits = 0;
    int trials = 100;

    /* Level 10 vs armor 5 with +5 weapon - should hit most of the time */
    for (int i = 0; i < trials; i++) {
        if (swing(10, 5, 5)) {
            hits++;
        }
    }

    assert_true(hits >= 60 && hits <= 100);
}

/* Test: swing() low level vs high armor */
static void test_swing_hard_hit(void **state) {
    (void) state; /* unused */
    int hits = 0;
    int trials = 100;

    /* Level 1 vs armor 8 with -5 weapon - should hit rarely
     * need = (20 - 1) - 8 = 11
     * hit if (res - 5) >= 11, i.e., res >= 16
     * This happens when res is 16-19, which is 4/20 = 20% */
    for (int i = 0; i < trials; i++) {
        if (swing(1, 8, -5)) {
            hits++;
        }
    }

    assert_true(hits >= 0 && hits <= 40);
}

int run_fight_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_swing_basic),
        cmocka_unit_test(test_swing_with_bonus),
        cmocka_unit_test(test_swing_easy_hit),
        cmocka_unit_test(test_swing_hard_hit),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
