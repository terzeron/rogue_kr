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
extern void init_rainbow(void);
extern char *rainbow[];

/* Test: pick_color without hallucination returns translated color */
static void test_pick_color_normal(void **state) {
    (void) state;

    player.t_flags &= ~ISHALU;
    const char *result = pick_color("blue");
    /* pick_color now returns translated message */
    assert_non_null(result);
    /* In test environment, msg_get returns the key, so check for "BLUE" or "blue" */
    assert_true(strstr(result, "BLUE") != NULL || strcmp(result, "blue") == 0);
}

/* Test: pick_color with ISHALU returns a random color from rainbow */
static void test_pick_color_hallucinating(void **state) {
    (void) state;

    /* Initialize rainbow array first */
    init_rainbow();

    player.t_flags |= ISHALU;
    const char *result = pick_color("blue");
    assert_non_null(result);
    assert_true(strlen(result) > 0);

    /* Result should be one of the rainbow colors */
    bool found = false;
    for (int i = 0; i < 27; i++) {
        if (rainbow[i] && strcmp(result, rainbow[i]) == 0) {
            found = true;
            break;
        }
    }
    assert_true(found);

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
