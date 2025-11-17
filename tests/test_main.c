/*
 * Master test runner - runs all test suites
 */

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <limits.h>

/* Minimal type definitions */
typedef struct { int x; int y; } coord;

/* Function declarations from main.c */
extern int rnd(int range);
extern int roll(int number, int sides);
extern int sign(int nm);
extern int spread(int nm);

/* External test suite runners */
extern int run_misc_tests(void);
extern int run_fight_tests(void);
extern int run_list_tests(void);
extern int run_move_tests(void);
extern int run_i18n_tests(void);
extern int run_armor_tests(void);
extern int run_pack_tests(void);
extern int run_potions_tests(void);
extern int run_rings_tests(void);
extern int run_scrolls_tests(void);
extern int run_sticks_tests(void);
extern int run_things_tests(void);
extern int run_weapons_tests(void);
extern int run_monsters_tests(void);
extern int run_daemon_tests(void);
extern int run_command_tests(void);
extern int run_daemons_tests(void);
extern int run_extern_tests(void);
extern int run_init_tests(void);
extern int run_io_tests(void);
extern int run_mach_dep_tests(void);
extern int run_mdport_tests(void);
extern int run_new_level_tests(void);
extern int run_options_tests(void);
extern int run_passages_tests(void);
extern int run_rip_tests(void);
extern int run_rooms_tests(void);
extern int run_save_tests(void);
extern int run_state_tests(void);
extern int run_utils_tests(void);
extern int run_vers_tests(void);
extern int run_wizard_tests(void);
extern int run_xcrypt_tests(void);

/* Test: rnd() returns 0 for range 0 */
static void test_rnd_zero_range(void **state) {
    (void) state; /* unused */
    int result = rnd(0);
    assert_int_equal(0, result);
}

/* Test: rnd() returns values in valid range */
static void test_rnd_valid_range(void **state) {
    (void) state; /* unused */
    for (int i = 0; i < 100; i++) {
        int result = rnd(10);
        assert_true(result >= 0 && result <= 9);
    }
}

/* Test: rnd() with different ranges */
static void test_rnd_various_ranges(void **state) {
    (void) state; /* unused */
    int val;

    val = rnd(1);
    assert_int_equal(0, val);  /* Only valid value */

    val = rnd(100);
    assert_true(val >= 0 && val <= 99);

    val = rnd(5);
    assert_true(val >= 0 && val <= 4);
}

/* Test: roll() with 0 dice */
static void test_roll_zero_dice(void **state) {
    (void) state; /* unused */
    int result = roll(0, 6);
    assert_int_equal(0, result);
}

/* Test: roll() 1d6 range */
static void test_roll_1d6(void **state) {
    (void) state; /* unused */
    for (int i = 0; i < 50; i++) {
        int result = roll(1, 6);
        assert_true(result >= 1 && result <= 6);
    }
}

/* Test: roll() 3d6 range */
static void test_roll_3d6(void **state) {
    (void) state; /* unused */
    for (int i = 0; i < 50; i++) {
        int result = roll(3, 6);
        assert_true(result >= 3 && result <= 18);
    }
}

/* Test: roll() various combinations */
static void test_roll_various(void **state) {
    (void) state; /* unused */
    int result;

    result = roll(1, 1);
    assert_int_equal(1, result);  /* Always 1 */

    result = roll(2, 4);
    assert_true(result >= 2 && result <= 8);

    result = roll(10, 10);
    assert_true(result >= 10 && result <= 100);
}

/* Test: sign() positive numbers */
static void test_sign_positive(void **state) {
    (void) state; /* unused */
    assert_int_equal(1, sign(1));
    assert_int_equal(1, sign(10));
    assert_int_equal(1, sign(999));
    assert_int_equal(1, sign(INT_MAX));
}

/* Test: sign() negative numbers */
static void test_sign_negative(void **state) {
    (void) state; /* unused */
    assert_int_equal(-1, sign(-1));
    assert_int_equal(-1, sign(-10));
    assert_int_equal(-1, sign(-999));
}

/* Test: sign() zero */
static void test_sign_zero(void **state) {
    (void) state; /* unused */
    assert_int_equal(0, sign(0));
}

/* Test: spread() produces reasonable values */
static void test_spread_basic(void **state) {
    (void) state; /* unused */
    for (int i = 0; i < 100; i++) {
        int result = spread(10);
        /* spread(n) should be roughly n/2 to 3n/2 */
        assert_true(result >= 0 && result <= 20);
    }
}

/* Test: spread() with different inputs */
static void test_spread_various(void **state) {
    (void) state; /* unused */
    int result;

    result = spread(0);
    assert_int_equal(0, result);

    result = spread(1);
    assert_true(result >= 0 && result <= 2);

    result = spread(100);
    assert_true(result >= 0 && result <= 150);
}

int main(void) {
    int failed = 0;

    printf("\n========================================\n");
    printf("Running Rogue Test Suite\n");
    printf("========================================\n\n");

    /* Run basic utility tests */
    printf("--- Main Utility Tests ---\n");
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_rnd_zero_range),
        cmocka_unit_test(test_rnd_valid_range),
        cmocka_unit_test(test_rnd_various_ranges),
        cmocka_unit_test(test_roll_zero_dice),
        cmocka_unit_test(test_roll_1d6),
        cmocka_unit_test(test_roll_3d6),
        cmocka_unit_test(test_roll_various),
        cmocka_unit_test(test_sign_positive),
        cmocka_unit_test(test_sign_negative),
        cmocka_unit_test(test_sign_zero),
        cmocka_unit_test(test_spread_basic),
        cmocka_unit_test(test_spread_various),
    };
    failed += cmocka_run_group_tests(tests, NULL, NULL);

    /* Run other test suites */
    printf("\n--- Misc Tests ---\n");
    failed += run_misc_tests();

    printf("\n--- Fight Tests ---\n");
    failed += run_fight_tests();

    printf("\n--- Monsters Tests ---\n");
    failed += run_monsters_tests();

    printf("\n--- Weapons Tests ---\n");
    failed += run_weapons_tests();

    printf("\n--- List Tests ---\n");
    failed += run_list_tests();

    printf("\n--- Move Tests ---\n");
    failed += run_move_tests();

    printf("\n--- Daemon Framework Tests ---\n");
    failed += run_daemon_tests();

    printf("\n--- Daemons Module Tests ---\n");
    failed += run_daemons_tests();

    printf("\n--- I18n Tests ---\n");
    failed += run_i18n_tests();

    printf("\n--- Armor Tests ---\n");
    failed += run_armor_tests();

    printf("\n--- Chase Tests (skipped - ch_ret is static) ---\n");

    printf("\n--- Pack Tests (skipped - requires curses) ---\n");

    printf("\n--- Potions Tests ---\n");
    failed += run_potions_tests();

    printf("\n--- Rings Tests ---\n");
    failed += run_rings_tests();

    printf("\n--- Scrolls Tests ---\n");
    failed += run_scrolls_tests();

    printf("\n--- Sticks Tests (skipped - requires curses) ---\n");

    printf("\n--- Things Tests (skipped - requires curses) ---\n");

    printf("\n--- Command Tests ---\n");
    failed += run_command_tests();

    printf("\n--- Extern Tests ---\n");
    failed += run_extern_tests();

    printf("\n--- Init Tests ---\n");
    failed += run_init_tests();

    printf("\n--- IO Tests ---\n");
    failed += run_io_tests();

    printf("\n--- Mach Dep Tests ---\n");
    failed += run_mach_dep_tests();

    printf("\n--- Mdport Tests ---\n");
    failed += run_mdport_tests();

    printf("\n--- New Level Tests ---\n");
    failed += run_new_level_tests();

    printf("\n--- Options Tests ---\n");
    failed += run_options_tests();

    printf("\n--- Passages Tests ---\n");
    failed += run_passages_tests();

    printf("\n--- RIP Tests ---\n");
    failed += run_rip_tests();

    printf("\n--- Rooms Tests ---\n");
    failed += run_rooms_tests();

    printf("\n--- Save Tests ---\n");
    failed += run_save_tests();

    printf("\n--- State Tests ---\n");
    failed += run_state_tests();

    printf("\n--- Utils Tests ---\n");
    failed += run_utils_tests();

    printf("\n--- Version Tests ---\n");
    failed += run_vers_tests();

    printf("\n--- Wizard Tests ---\n");
    failed += run_wizard_tests();

    printf("\n--- Xcrypt Tests ---\n");
    failed += run_xcrypt_tests();

    printf("\n========================================\n");
    if (failed == 0) {
        printf("All tests PASSED!\n");
    } else {
        printf("Some tests FAILED! (total failures: %d)\n", failed);
    }
    printf("\n========================================\n\n");

    return failed;
}
