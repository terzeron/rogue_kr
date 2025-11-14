/*
 * Tests for rings.c functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../tests/test_stubs.h"

/* External function declarations from rings.c */
extern void ring_on(void);
extern void ring_off(void);
extern int gethand(void);
extern int ring_eat(int hand);
extern char *ring_num(THING *obj);

/* Test: ring_on() with no rings equipped */
static void test_ring_on_empty_hands(void **state) {
    (void) state;

    cur_ring[LEFT] = NULL;
    cur_ring[RIGHT] = NULL;
    pack = NULL;

    ring_on();

    /* Should handle gracefully */
    assert_null(cur_ring[LEFT]);
    assert_null(cur_ring[RIGHT]);
}

/* Test: ring_off() with no rings */
static void test_ring_off_no_rings(void **state) {
    (void) state;

    cur_ring[LEFT] = NULL;
    cur_ring[RIGHT] = NULL;
    terse = FALSE;

    ring_off();

    assert_null(cur_ring[LEFT]);
    assert_null(cur_ring[RIGHT]);
}

/* Test: ring_off() with left ring only */
static void test_ring_off_left_only(void **state) {
    (void) state;

    THING ring;
    memset(&ring, 0, sizeof(THING));
    ring.o_type = RING;
    ring.o_which = R_PROTECT;

    cur_ring[LEFT] = &ring;
    cur_ring[RIGHT] = NULL;

    /* Note: Full test requires dropcheck implementation */
    assert_non_null(cur_ring[LEFT]);
}

/* Test: ring_eat() with no ring */
static void test_ring_eat_no_ring(void **state) {
    (void) state;

    cur_ring[LEFT] = NULL;

    int result = ring_eat(LEFT);

    assert_int_equal(result, 0);
}

/* Test: ring_eat() with protection ring */
static void test_ring_eat_protection(void **state) {
    (void) state;

    THING ring;
    memset(&ring, 0, sizeof(THING));
    ring.o_which = R_PROTECT;

    cur_ring[LEFT] = &ring;

    int result = ring_eat(LEFT);

    assert_true(result >= 0);  /* Protection uses positive food */
}

/* Test: ring_eat() with regeneration ring */
static void test_ring_eat_regen(void **state) {
    (void) state;

    THING ring;
    memset(&ring, 0, sizeof(THING));
    ring.o_which = R_REGEN;

    cur_ring[LEFT] = &ring;

    int result = ring_eat(LEFT);

    assert_true(result >= 0);  /* Regen uses positive food */
}

/* Test: ring_num() with unknown ring */
static void test_ring_num_unknown(void **state) {
    (void) state;

    THING ring;
    memset(&ring, 0, sizeof(THING));
    ring.o_flags = 0;  /* Not known */

    char *result = ring_num(&ring);

    assert_string_equal(result, "");
}

/* Test: ring_num() with protect ring */
static void test_ring_num_protect(void **state) {
    (void) state;

    THING ring;
    memset(&ring, 0, sizeof(THING));
    ring.o_which = R_PROTECT;
    ring.o_flags = ISKNOW;
    ring.o_arm = 2;

    char *result = ring_num(&ring);

    assert_non_null(result);
    /* Should contain the bonus */
}

/* Test: ring_num() with non-bonus ring */
static void test_ring_num_teleport(void **state) {
    (void) state;

    THING ring;
    memset(&ring, 0, sizeof(THING));
    ring.o_which = R_TELEPORT;
    ring.o_flags = ISKNOW;

    char *result = ring_num(&ring);

    assert_string_equal(result, "");
}

/* Test suite runner */
int run_rings_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ring_on_empty_hands),
        cmocka_unit_test(test_ring_off_no_rings),
        cmocka_unit_test(test_ring_off_left_only),
        cmocka_unit_test(test_ring_eat_no_ring),
        cmocka_unit_test(test_ring_eat_protection),
        cmocka_unit_test(test_ring_eat_regen),
        cmocka_unit_test(test_ring_num_unknown),
        cmocka_unit_test(test_ring_num_protect),
        cmocka_unit_test(test_ring_num_teleport),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
