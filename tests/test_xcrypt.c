/*
 * Tests for xcrypt.c password hashing helper
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

extern char *xcrypt(const char *key, const char *setting);

/* Test: xcrypt returns 13-character string starting with salt */
static void test_xcrypt_basic(void **state) {
    (void) state;

    const char *salt = "ab";
    char *hash = xcrypt("password", salt);
    assert_non_null(hash);
    assert_int_equal(13, (int)strlen(hash));
    assert_int_equal('a', hash[0]);
    assert_int_equal('b', hash[1]);
}

int run_xcrypt_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_xcrypt_basic),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
