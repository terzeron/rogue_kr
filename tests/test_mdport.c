/*
 * Tests for mdport.c platform helpers
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "rogue.h"

extern int md_getpid(void);
extern char *md_getusername(void);
extern char *md_gethomedir(void);
extern void md_sleep(int s);
extern char *md_getshell(void);

/* Test: md_getpid() returns a positive value */
static void test_md_getpid_positive(void **state) {
    (void) state;
    assert_true(md_getpid() > 0);
}

/* Test: md_getusername() returns a non-empty string */
static void test_md_getusername_nonempty(void **state) {
    (void) state;
    char *name = md_getusername();
    assert_non_null(name);
    assert_true(strlen(name) > 0);
}

/* Test: md_gethomedir() honors HOME variable */
static void test_md_gethomedir_returns_path(void **state) {
    (void) state;

    char *dir = md_gethomedir();
    assert_non_null(dir);
    size_t len = strlen(dir);
    assert_true(len > 0);
    char last = dir[len - 1];
    assert_true(last == '/' || last == '\\');
}

/* Test: md_sleep(0) returns quickly */
static void test_md_sleep_zero(void **state) {
    (void) state;
    md_sleep(0);
    assert_true(1);
}

/* Test: md_getshell() returns a fallback shell */
static void test_md_getshell_nonempty(void **state) {
    (void) state;
    char *shell = md_getshell();
    assert_non_null(shell);
    assert_true(strlen(shell) > 0);
}

int run_mdport_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_md_getpid_positive),
        cmocka_unit_test(test_md_getusername_nonempty),
        cmocka_unit_test(test_md_gethomedir_returns_path),
        cmocka_unit_test(test_md_sleep_zero),
        cmocka_unit_test(test_md_getshell_nonempty),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
