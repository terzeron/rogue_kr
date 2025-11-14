/*
 * Tests for init.c helpers
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdbool.h>
#include <string.h>
#include <curses.h>
#include "rogue.h"

extern char *pick_color(char *col);
extern void sumprobs(struct obj_info *info, int bound);

/* Test: pick_color without hallucination returns original */
static void test_pick_color_normal(void **state) {
    (void) state;

    player.t_flags &= ~ISHALU;
    char sample[] = "blue";
    assert_string_equal(sample, pick_color(sample));
}

/* Test: pick_color with ISHALU returns a non-empty string */
static void test_pick_color_hallucinating(void **state) {
    (void) state;

    player.t_flags |= ISHALU;
    const char *result = pick_color("blue");
    assert_non_null(result);
    assert_true(strlen(result) > 0);
    player.t_flags &= ~ISHALU;
}

/* Test: sumprobs produces cumulative probabilities */
static void test_sumprobs_accumulates(void **state) {
    (void) state;

    struct obj_info info[3] = {
        {"one", 20, 0, NULL, FALSE},
        {"two", 30, 0, NULL, FALSE},
        {"three", 50, 0, NULL, FALSE},
    };

    sumprobs(info, 3);

    assert_int_equal(20, info[0].oi_prob);
    assert_int_equal(50, info[1].oi_prob);
    assert_int_equal(100, info[2].oi_prob);
}

int run_init_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_pick_color_normal),
        cmocka_unit_test(test_pick_color_hallucinating),
        cmocka_unit_test(test_sumprobs_accumulates),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
