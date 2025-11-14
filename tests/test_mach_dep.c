/*
 * Tests for mach_dep.c helpers
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "rogue.h"

extern bool is_symlink(char *sp);

/* Test: is_symlink() detects regular files */
static void test_is_symlink_regular_file(void **state) {
    (void) state;

    char path[] = "/tmp/rogue_fileXXXXXX";
    int fd = mkstemp(path);
    assert_true(fd >= 0);
    close(fd);

    assert_false(is_symlink(path));

    unlink(path);
}

/* Test: is_symlink() detects symbolic links */
static void test_is_symlink_link(void **state) {
    (void) state;

    char target[] = "/tmp/rogue_targetXXXXXX";
    int fd = mkstemp(target);
    assert_true(fd >= 0);
    close(fd);

    char linkpath[sizeof(target) + 5];
    snprintf(linkpath, sizeof(linkpath), "%s_l", target);

    assert_return_code(symlink(target, linkpath), 0);
    assert_true(is_symlink(linkpath));

    unlink(linkpath);
    unlink(target);
}

int run_mach_dep_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_is_symlink_regular_file),
        cmocka_unit_test(test_is_symlink_link),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
