/*
 * Tests for misc.c functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

/* Include curses and rogue.h for correct type definitions */
#include <curses.h>
#include "rogue.h"

/* Function declarations from misc.c that are not in rogue.h */
extern int sign(int nm);
extern int spread(int nm);

/* Test: sign() edge cases */
static void test_sign_edge_cases(void **state) {
    (void) state; /* unused */
    assert_int_equal(0, sign(0));
    assert_int_equal(1, sign(INT_MAX));
    assert_int_equal(-1, sign(INT_MIN));
}

/* Test: sign() boundary values */
static void test_sign_boundaries(void **state) {
    (void) state; /* unused */
    assert_int_equal(1, sign(1));
    assert_int_equal(-1, sign(-1));
    assert_int_equal(1, sign(2));
    assert_int_equal(-1, sign(-2));
}

/* Test: spread() consistency */
static void test_spread_consistency(void **state) {
    (void) state; /* unused */
    /* Multiple calls should produce varied results */
    int results[10];
    for (int i = 0; i < 10; i++) {
        results[i] = spread(10);
    }

    /* At least some variation should exist */
    int all_same = 1;
    for (int i = 1; i < 10; i++) {
        if (results[i] != results[0]) {
            all_same = 0;
            break;
        }
    }
    assert_false(all_same);  /* Should have some variation */
}

/* Test: spread() zero input */
static void test_spread_zero(void **state) {
    (void) state; /* unused */
    int result = spread(0);
    assert_int_equal(0, result);
}

/* Test: spread() large values */
static void test_spread_large(void **state) {
    (void) state; /* unused */
    int result = spread(1000);
    assert_true(result >= 0 && result <= 1500);
}

/* Test: vowelstr() with consonant */
static void test_vowelstr_consonant(void **state) {
    (void) state; /* unused */
    char *result = vowelstr("sword");
    assert_string_equal("", result);
}

/* Test: vowelstr() with vowel */
static void test_vowelstr_vowel(void **state) {
    (void) state; /* unused */
    char *result = vowelstr("apple");
    assert_string_equal("n", result);
}

/* Test: vowelstr() various inputs */
static void test_vowelstr_various(void **state) {
    (void) state; /* unused */
    assert_string_equal("n", vowelstr("orange"));
    assert_string_equal("", vowelstr("banana"));
    assert_string_equal("n", vowelstr("iron"));
    assert_string_equal("", vowelstr("gold"));
}

/* Test: choose_str() when hallucinating */
static void test_choose_str_terse(void **state) {
    (void) state; /* unused */
    player.t_flags = ISHALU;
    const char *result = choose_str("hallucinating message", "normal message");
    assert_string_equal("hallucinating message", result);
}

/* Test: choose_str() when not hallucinating */
static void test_choose_str_normal(void **state) {
    (void) state; /* unused */
    player.t_flags = 0;
    const char *result = choose_str("hallucinating message", "normal message");
    assert_string_equal("normal message", result);
}

/* Test: add_str() with positive amount */
static void test_add_str_positive(void **state) {
    (void) state; /* unused */
    str_t strength = 16;
    add_str(&strength, 5);
    assert_int_equal(21, strength);
}

/* Test: add_str() with negative amount */
static void test_add_str_negative(void **state) {
    (void) state; /* unused */
    str_t strength = 16;
    add_str(&strength, -5);
    assert_int_equal(11, strength);
}

/* Test: add_str() lower bound (minimum 3) */
static void test_add_str_lower_bound(void **state) {
    (void) state; /* unused */
    str_t strength = 4;
    add_str(&strength, -2);
    assert_int_equal(3, strength);
}

/* Test: add_str() upper bound (maximum 31) */
static void test_add_str_upper_bound(void **state) {
    (void) state; /* unused */
    str_t strength = 28;
    add_str(&strength, 10);
    assert_int_equal(31, strength);
}

/* Test: is_current() with NULL */
static void test_is_current_null(void **state) {
    (void) state; /* unused */
    bool result = is_current(NULL);
    assert_false(result);
}

/* Test: is_current() with current armor */
static void test_is_current_armor(void **state) {
    (void) state; /* unused */
    THING armor_item = {0};
    THING *old_armor = cur_armor;

    cur_armor = &armor_item;
    bool result = is_current(&armor_item);
    assert_true(result);

    cur_armor = old_armor;
}

/* Test: is_current() with non-current item */
static void test_is_current_not_equipped(void **state) {
    (void) state; /* unused */
    THING item = {0};
    THING *old_armor = cur_armor;
    THING *old_weapon = cur_weapon;
    THING *old_ring0 = cur_ring[0];
    THING *old_ring1 = cur_ring[1];

    cur_armor = NULL;
    cur_weapon = NULL;
    cur_ring[0] = NULL;
    cur_ring[1] = NULL;

    bool result = is_current(&item);
    assert_false(result);

    cur_armor = old_armor;
    cur_weapon = old_weapon;
    cur_ring[0] = old_ring0;
    cur_ring[1] = old_ring1;
}

/* Test: rnd_thing() returns valid thing character */
static void test_rnd_thing_valid(void **state) {
    (void) state; /* unused */
    int old_level = level;
    level = 10;

    char thing = rnd_thing();

    /* Check if it's one of the valid thing types */
    bool valid = (thing == POTION || thing == SCROLL || thing == RING ||
                  thing == STICK || thing == FOOD || thing == WEAPON ||
                  thing == ARMOR || thing == STAIRS || thing == GOLD);
    assert_true(valid);

    level = old_level;
}

/* Test: rnd_thing() variety across multiple calls */
static void test_rnd_thing_variety(void **state) {
    (void) state; /* unused */
    int old_level = level;
    level = 10;

    char things[20];
    for (int i = 0; i < 20; i++) {
        things[i] = rnd_thing();
    }

    /* Check that we get some variety (not all the same) */
    bool has_variety = false;
    for (int i = 1; i < 20; i++) {
        if (things[i] != things[0]) {
            has_variety = true;
            break;
        }
    }
    assert_true(has_variety);

    level = old_level;
}

int run_misc_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sign_edge_cases),
        cmocka_unit_test(test_sign_boundaries),
        cmocka_unit_test(test_spread_consistency),
        cmocka_unit_test(test_spread_zero),
        cmocka_unit_test(test_spread_large),
        cmocka_unit_test(test_vowelstr_consonant),
        cmocka_unit_test(test_vowelstr_vowel),
        cmocka_unit_test(test_vowelstr_various),
        cmocka_unit_test(test_choose_str_terse),
        cmocka_unit_test(test_choose_str_normal),
        cmocka_unit_test(test_add_str_positive),
        cmocka_unit_test(test_add_str_negative),
        cmocka_unit_test(test_add_str_lower_bound),
        cmocka_unit_test(test_add_str_upper_bound),
        cmocka_unit_test(test_is_current_null),
        cmocka_unit_test(test_is_current_armor),
        cmocka_unit_test(test_is_current_not_equipped),
        cmocka_unit_test(test_rnd_thing_valid),
        cmocka_unit_test(test_rnd_thing_variety),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
