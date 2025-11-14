/*
 * Tests for options.c option table
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <curses.h>
#include <string.h>
#include "rogue.h"

/* Re-declare OPTION structure for access */
typedef struct optstruct {
    char *o_name;
    char *o_prompt;
    void *o_opt;
    void (*o_putfunc)(void *opt);
    int (*o_getfunc)(void *opt, WINDOW *win);
} OPTION;

extern OPTION optlist[];

/* Test: first option controls terse flag */
static void test_option_terse_entry(void **state) {
    (void) state;

    assert_string_equal("terse", optlist[0].o_name);
    assert_ptr_equal(&terse, optlist[0].o_opt);
}

/* Test: inventory option references inv_type */
static void test_option_inventory_entry(void **state) {
    (void) state;

    assert_string_equal("inven", optlist[6].o_name);
    assert_ptr_equal(&inv_type, optlist[6].o_opt);
}

int run_options_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_option_terse_entry),
        cmocka_unit_test(test_option_inventory_entry),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
