/*
 * Tests for save.c encryption helpers
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "rogue.h"

/* Test: encwrite/encread perform round trip encryption */
static void test_encwrite_encread_roundtrip(void **state) {
    (void) state;

    FILE *fp = tmpfile();
    assert_non_null(fp);

    char original[] = "testing123";
    size_t len = strlen(original);
    assert_int_equal((int)len, (int)encwrite(original, len, fp));

    rewind(fp);

    char buffer[sizeof(original)];
    memset(buffer, 0, sizeof(buffer));
    assert_int_equal((int)len, (int)encread(buffer, len, fp));
    buffer[len] = '\0';

    assert_string_equal("testing123", buffer);

    fclose(fp);
}

int run_save_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_encwrite_encread_roundtrip),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
