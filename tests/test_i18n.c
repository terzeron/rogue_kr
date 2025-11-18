/*
 * test_i18n.c - Unit tests for internationalization (i18n) system
 *
 * Tests the message catalog loading and retrieval functionality
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

/* Include i18n header */
#include "../i18n.h"

/*
 * Test: i18n_init should succeed
 */
static void test_i18n_init_success(void **state) {
    (void) state; /* unused */
    int result = i18n_init();
    assert_int_equal(result, 0);
}

/*
 * Test: msg_get should return English message for known key
 */
static void test_msg_get_known_key(void **state) {
    (void) state; /* unused */
    const char *msg = msg_get("MSG_MORE");
    assert_non_null(msg);
    assert_true(strlen(msg) > 0);
}

/*
 * Test: msg_get should return key itself for unknown key
 */
static void test_msg_get_unknown_key(void **state) {
    (void) state; /* unused */
    const char *msg = msg_get("MSG_NONEXISTENT_KEY_12345");
    assert_non_null(msg);
    assert_string_equal(msg, "MSG_NONEXISTENT_KEY_12345");
}

/*
 * Test: msg_get should handle NULL key gracefully
 */
static void test_msg_get_null_key(void **state) {
    (void) state; /* unused */
    const char *msg = msg_get(NULL);
    assert_non_null(msg);
    assert_string_equal(msg, "");
}

/*
 * Test: msg_get should handle empty key
 */
static void test_msg_get_empty_key(void **state) {
    (void) state; /* unused */
    const char *msg = msg_get("");
    assert_non_null(msg);
    assert_string_equal(msg, "");
}

/*
 * Test: Verify specific English messages
 */
static void test_english_messages(void **state) {
    (void) state; /* unused */
    /* Set English locale for testing */
    setenv("LANG", "en_US.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* io.c messages */
    assert_string_equal(msg_get("MSG_MORE"), "--More--");
    assert_string_equal(msg_get("MSG_LEVEL"), "Level");
    assert_string_equal(msg_get("MSG_GOLD"), "Gold");
    assert_string_equal(msg_get("MSG_HP"), "Hp");
    assert_string_equal(msg_get("MSG_STR"), "Str");
    assert_string_equal(msg_get("MSG_ARM"), "Arm");
    assert_string_equal(msg_get("MSG_EXP"), "Exp");
    assert_string_equal(msg_get("MSG_HUNGRY"), "Hungry");
    assert_string_equal(msg_get("MSG_WEAK"), "Weak");
    assert_string_equal(msg_get("MSG_FAINT"), "Faint");
}

/*
 * Test: Verify command.c messages
 */
static void test_command_messages(void **state) {
    (void) state; /* unused */
    /* Set English locale for testing */
    setenv("LANG", "en_US.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    assert_string_equal(msg_get("MSG_YOU_CAN_MOVE_AGAIN"), "you can move again");
    assert_string_equal(msg_get("MSG_THERE_IS"), "there is");
    assert_string_equal(msg_get("MSG_NOTHING_HERE"), "nothing here");
    assert_string_equal(msg_get("MSG_VERSION"), "version %s. (mctesq was here)");
    assert_string_equal(msg_get("MSG_ILLEGAL_COMMAND"), "illegal command '%s'");
}

/*
 * Test: Verify identify.c messages
 */
static void test_identify_messages(void **state) {
    (void) state; /* unused */
    /* Set English locale for testing */
    setenv("LANG", "en_US.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    assert_string_equal(msg_get("MSG_WALL"), "wall of a room");
    assert_string_equal(msg_get("MSG_GOLD_DESC"), "gold");
    assert_string_equal(msg_get("MSG_STAIRCASE"), "a staircase");
    assert_string_equal(msg_get("MSG_DOOR"), "door");
    assert_string_equal(msg_get("MSG_YOU"), "you");
    assert_string_equal(msg_get("MSG_POTION"), "potion");
    assert_string_equal(msg_get("MSG_SCROLL"), "scroll");
    assert_string_equal(msg_get("MSG_WEAPON"), "weapon");
    assert_string_equal(msg_get("MSG_ARMOR"), "armor");
    assert_string_equal(msg_get("MSG_AMULET"), "the Amulet of Yendor");
}

/*
 * Test: msg_get should return same pointer for repeated calls
 */
static void test_msg_get_consistency(void **state) {
    (void) state; /* unused */
    const char *msg1 = msg_get("MSG_MORE");
    const char *msg2 = msg_get("MSG_MORE");
    assert_ptr_equal(msg1, msg2);  /* Should be same pointer */
}

/*
 * Test: Multiple init calls should be safe
 */
static void test_multiple_init(void **state) {
    (void) state; /* unused */
    int result1 = i18n_init();
    int result2 = i18n_init();
    assert_int_equal(result1, 0);
    assert_int_equal(result2, 0);

    /* Messages should still work */
    const char *msg = msg_get("MSG_MORE");
    assert_non_null(msg);
}

/*
 * Test: Cleanup should be safe
 */
static void test_cleanup(void **state) {
    (void) state; /* unused */
    i18n_cleanup();
    /* Multiple cleanup should be safe */
    i18n_cleanup();
}

/*
 * Test: After cleanup and reinit, messages should work
 */
static void test_cleanup_and_reinit(void **state) {
    (void) state; /* unused */
    i18n_cleanup();
    int result = i18n_init();
    assert_int_equal(result, 0);

    const char *msg = msg_get("MSG_MORE");
    assert_non_null(msg);
    assert_true(strlen(msg) > 0);
}

int run_i18n_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_i18n_init_success),
        cmocka_unit_test(test_msg_get_known_key),
        cmocka_unit_test(test_msg_get_unknown_key),
        cmocka_unit_test(test_msg_get_null_key),
        cmocka_unit_test(test_msg_get_empty_key),
        cmocka_unit_test(test_english_messages),
        cmocka_unit_test(test_command_messages),
        cmocka_unit_test(test_identify_messages),
        cmocka_unit_test(test_msg_get_consistency),
        cmocka_unit_test(test_multiple_init),
        cmocka_unit_test(test_cleanup),
        cmocka_unit_test(test_cleanup_and_reinit),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
