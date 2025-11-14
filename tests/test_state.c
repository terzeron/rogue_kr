/*
 * Tests for state.c serialization helpers
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "rogue.h"

extern int rs_write_int(FILE *savef, int c);
extern int rs_read_int(FILE *inf, int *i);
extern int rs_write_char(FILE *savef, char c);
extern int rs_read_char(FILE *inf, char *c);
extern int rs_write_string(FILE *savef, char *s);
extern int rs_read_string(FILE *inf, char *s, int max);

/* Test: rs_write_int and rs_read_int round trip */
static void test_rs_int_roundtrip(void **state) {
    (void) state;

    FILE *fp = tmpfile();
    assert_non_null(fp);

    int value = 123456789;
    rs_write_int(fp, value);
    rewind(fp);

    int read_value = 0;
    rs_read_int(fp, &read_value);
    assert_int_equal(value, read_value);

    fclose(fp);
}

/* Test: rs_write_char/rs_read_char round trip */
static void test_rs_char_roundtrip(void **state) {
    (void) state;

    FILE *fp = tmpfile();
    assert_non_null(fp);

    rs_write_char(fp, 'Z');
    rewind(fp);

    char c = '\0';
    rs_read_char(fp, &c);
    assert_int_equal('Z', c);

    fclose(fp);
}

/* Test: rs_write_string/rs_read_string round trip */
static void test_rs_string_roundtrip(void **state) {
    (void) state;

    FILE *fp = tmpfile();
    assert_non_null(fp);

    char sample[] = "hello world";
    rs_write_string(fp, sample);
    rewind(fp);

    char buffer[32];
    memset(buffer, 0, sizeof(buffer));
    rs_read_string(fp, buffer, sizeof(buffer));
    assert_string_equal(sample, buffer);

    fclose(fp);
}

int run_state_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_rs_int_roundtrip),
        cmocka_unit_test(test_rs_char_roundtrip),
        cmocka_unit_test(test_rs_string_roundtrip),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
