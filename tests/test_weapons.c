/*
 * Tests for weapons.c functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <curses.h>
#include "rogue.h"

/* Constants */
#define MAXWEAPONS 9

/* External declarations */
extern char *num(int n1, int n2, char type);
extern void init_weapon(THING *weap, int which);

/* Test: weapon data exists */
static void test_weapon_data_exists(void **state) {
    (void) state; /* unused */
    /* Check that weapon info is populated */
    assert_non_null(weap_info);
}

/* Test: weapon names are valid */
static void test_weapon_names_valid(void **state) {
    (void) state; /* unused */
    /* Check first few weapons */
    for (int i = 0; i < 5; i++) {
        assert_non_null(weap_info[i].oi_name);
        assert_true(strlen(weap_info[i].oi_name) > 0);
    }
}

/* Test: weapon probabilities are reasonable */
static void test_weapon_probabilities(void **state) {
    (void) state; /* unused */
    /* Probabilities should be non-negative */
    for (int i = 0; i < MAXWEAPONS; i++) {
        assert_true(weap_info[i].oi_prob >= 0);
    }
}

/* Test: num() with positive values for weapon */
static void test_num_positive_weapon(void **state) {
    (void) state; /* unused */
    char *result = num(3, 4, WEAPON);
    assert_string_equal("+3,+4", result);
}

/* Test: num() with negative values for weapon */
static void test_num_negative_weapon(void **state) {
    (void) state; /* unused */
    char *result = num(-2, -1, WEAPON);
    assert_string_equal("-2,-1", result);
}

/* Test: num() with mixed values for weapon */
static void test_num_mixed_weapon(void **state) {
    (void) state; /* unused */
    char *result = num(2, -3, WEAPON);
    assert_string_equal("+2,-3", result);
}

/* Test: num() for armor (single value) */
static void test_num_armor(void **state) {
    (void) state; /* unused */
    char *result = num(5, 0, ARMOR);
    assert_string_equal("+5", result);
}

/* Test: num() for negative armor */
static void test_num_armor_negative(void **state) {
    (void) state; /* unused */
    char *result = num(-3, 0, ARMOR);
    assert_string_equal("-3", result);
}

/* Test: init_weapon() initializes mace */
static void test_init_weapon_mace(void **state) {
    (void) state; /* unused */
    THING weapon = {0};

    init_weapon(&weapon, MACE);

    assert_int_equal(WEAPON, weapon.o_type);
    assert_int_equal(MACE, weapon.o_which);
    assert_int_equal(0, weapon.o_hplus);
    assert_int_equal(0, weapon.o_dplus);
    /* Damage string should be populated */
    assert_true(strlen(weapon.o_damage) > 0);
}

/* Test: init_weapon() initializes dagger with count */
static void test_init_weapon_dagger(void **state) {
    (void) state; /* unused */
    THING weapon = {0};

    init_weapon(&weapon, DAGGER);

    assert_int_equal(WEAPON, weapon.o_type);
    assert_int_equal(DAGGER, weapon.o_which);
    /* Daggers should have count 2-5 */
    assert_true(weapon.o_count >= 2 && weapon.o_count <= 5);
}

/* Test: init_weapon() initializes arrow with count */
static void test_init_weapon_arrow(void **state) {
    (void) state; /* unused */
    THING weapon = {0};

    init_weapon(&weapon, ARROW);

    assert_int_equal(WEAPON, weapon.o_type);
    assert_int_equal(ARROW, weapon.o_which);
    /* Arrows should have count 10-20 */
    assert_true(weapon.o_count >= 10 && weapon.o_count <= 20);
}

/* Test: init_weapon() for various weapons */
static void test_init_weapon_variety(void **state) {
    (void) state; /* unused */
    THING weapon;

    /* Test each weapon type */
    for (int i = 0; i < MAXWEAPONS; i++) {
        memset(&weapon, 0, sizeof(THING));
        init_weapon(&weapon, i);

        assert_int_equal(WEAPON, weapon.o_type);
        assert_int_equal(i, weapon.o_which);
        assert_true(strlen(weapon.o_damage) > 0);
    }
}

int run_weapons_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_weapon_data_exists),
        cmocka_unit_test(test_weapon_names_valid),
        cmocka_unit_test(test_weapon_probabilities),
        cmocka_unit_test(test_num_positive_weapon),
        cmocka_unit_test(test_num_negative_weapon),
        cmocka_unit_test(test_num_mixed_weapon),
        cmocka_unit_test(test_num_armor),
        cmocka_unit_test(test_num_armor_negative),
        cmocka_unit_test(test_init_weapon_mace),
        cmocka_unit_test(test_init_weapon_dagger),
        cmocka_unit_test(test_init_weapon_arrow),
        cmocka_unit_test(test_init_weapon_variety),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
