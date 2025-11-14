/*
 * Tests for utils.c math helpers
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <curses.h>
#include "rogue.h"

/* utils.c 에 정의된 함수들 */
extern int rnd(int range);
extern int roll(int number, int sides);
extern int sign(int nm);
extern int spread(int nm);

/* ce 매크로는 rogue.h에 정의되어 있습니다. */
#define ce(a,b) ((a).x == (b).x && (a).y == (b).y)

/* Test: dist()가 동일한 점에 대해 0을 반환하는지 확인 */
static void test_dist_same_point_is_zero(void **state) {
    (void) state;
    assert_int_equal(0, dist(10, 20, 10, 20));
}

/* Test: dist()가 수평, 수직, 대각선 거리를 올바르게 계산하는지 확인 (체비쇼프 거리) */
static void test_dist_calculates_chebyshev_distance(void **state) {
    (void) state;
    /* 수평 */
    assert_int_equal(5, dist(5, 5, 5, 10));
    /* 수직 */
    assert_int_equal(5, dist(5, 5, 10, 5));
    /* 대각선 (dx, dy 중 큰 값) */
    assert_int_equal(4, dist(0, 0, 3, 4));
    assert_int_equal(5, dist(10, 10, 15, 12));
}

/* Test: dist()가 음수 좌표를 올바르게 처리하는지 확인 */
static void test_dist_handles_negative_coordinates(void **state) {
    (void) state;
    assert_int_equal(10, dist(-5, -5, 5, 5));
    assert_int_equal(20, dist(-10, 0, 10, 0));
}

/* Test: dist_cp()가 dist()와 동일한 결과를 반환하는지 확인 */
static void test_dist_cp_matches_dist(void **state) {
    (void) state;
    coord c1 = {20, 10}, c2 = {25, 18};
    assert_int_equal(dist(c1.y, c1.x, c2.y, c2.x), dist_cp(&c1, &c2));
}

/* Test: ce()가 동일한 좌표에 대해 true를 반환하는지 확인 */
static void test_ce_returns_true_for_equal_coords(void **state) {
    (void) state;
    coord c1 = {10, 20};
    coord c2 = {10, 20};
    assert_true(ce(c1, c2));
}

/* Test: ce()가 다른 좌표에 대해 false를 반환하는지 확인 */
static void test_ce_returns_false_for_unequal_coords(void **state) {
    (void) state;
    coord c1 = {10, 20};
    coord c2 = {11, 20}; /* x가 다름 */
    coord c3 = {10, 21}; /* y가 다름 */
    assert_false(ce(c1, c2));
    assert_false(ce(c1, c3));
}

/* Test: rnd()가 0 또는 양수 범위 내의 값을 반환하는지 확인 */
static void test_rnd_returns_in_range(void **state) {
    (void) state;
    assert_int_equal(0, rnd(0));
    for (int i = 0; i < 100; i++) {
        int val = rnd(10);
        assert_in_range(val, 0, 9);
    }
}

/* Test: roll()이 주사위 굴림의 합계를 올바르게 계산하는지 확인 */
static void test_roll_calculates_sum(void **state) {
    (void) state;
    assert_int_equal(0, roll(0, 6)); /* 0개의 주사위 */
    for (int i = 0; i < 100; i++) {
        int val = roll(3, 6); /* 3d6 */
        assert_in_range(val, 3, 18);
    }
}

/* Test: sign()이 양수, 음수, 0에 대해 올바른 부호를 반환하는지 확인 */
static void test_sign_returns_correct_sign(void **state) {
    (void) state;
    assert_int_equal(1, sign(123));
    assert_int_equal(-1, sign(-456));
    assert_int_equal(0, sign(0));
}

/* Test: spread()가 주어진 값 주변의 분포된 값을 반환하는지 확인 */
static void test_spread_returns_distributed_value(void **state) {
    (void) state;
    int base = 10;
    for (int i = 0; i < 100; i++) {
        int val = spread(base);
        /* spread(10)는 5 + rnd(10) 이므로 5에서 14 사이의 값을 반환합니다. */
        assert_in_range(val, 5, 14);
    }
}

int run_utils_tests(void) {
    const struct CMUnitTest tests[] = {
        /* dist() and ce() tests */
        cmocka_unit_test(test_dist_same_point_is_zero),
        cmocka_unit_test(test_dist_calculates_chebyshev_distance),
        cmocka_unit_test(test_dist_handles_negative_coordinates),
        cmocka_unit_test(test_dist_cp_matches_dist),
        cmocka_unit_test(test_ce_returns_true_for_equal_coords),
        cmocka_unit_test(test_ce_returns_false_for_unequal_coords),
        /* rnd(), roll(), sign(), spread() tests */
        cmocka_unit_test(test_rnd_returns_in_range),
        cmocka_unit_test(test_roll_calculates_sum),
        cmocka_unit_test(test_sign_returns_correct_sign),
        cmocka_unit_test(test_spread_returns_distributed_value),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
