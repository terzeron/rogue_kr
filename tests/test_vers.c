/*
 * Tests for vers.c version strings
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

extern char *release;
extern char encstr[];
extern char statlist[];
extern char version[];

/* Test: version metadata strings are not empty */
static void test_version_strings(void **state) {
    (void) state;

    assert_non_null(release);
    assert_true(strlen(release) > 0);
    assert_true(strlen(version) > 0);
    assert_true(strlen(encstr) > 0);
    assert_true(strlen(statlist) > 0);
}

int run_vers_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_version_strings),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
