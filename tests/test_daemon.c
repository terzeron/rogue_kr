/*
 * Tests for daemon.c - Event system (daemons and fuses)
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>

/* Daemon system constants */
#define EMPTY 0
#define DAEMON -1
#define MAXDAEMONS 20
#define BEFORE 1
#define AFTER 2

/* Delayed action structure from daemon.c */
struct delayed_action {
    int d_type;
    void (*d_func)();
    int d_arg;
    int d_time;
};

/* External declarations */
extern struct delayed_action d_list[MAXDAEMONS];
extern void start_daemon(void (*func)(), int arg, int type);
extern void kill_daemon(void (*func)());
extern void do_daemons(int flag);
extern void fuse(void (*func)(), int arg, int time, int type);
extern void extinguish(void (*func)());
extern void lengthen(void (*func)(), int xtime);
extern void do_fuses(int flag);

/* Test counters for callbacks */
static int test_counter_1 = 0;
static int test_counter_2 = 0;
static int test_arg_received = 0;

/* Test callback functions */
void test_daemon_func_1(int arg) {
    test_counter_1++;
    test_arg_received = arg;
}

void test_daemon_func_2(int arg) {
    test_counter_2++;
}

void test_fuse_func(int arg) {
    test_counter_1 += 10;
    test_arg_received = arg;
}

/* Helper to clear all daemons/fuses */
void clear_daemon_list() {
    for (int i = 0; i < MAXDAEMONS; i++) {
        d_list[i].d_type = EMPTY;
    }
}

/* Helper to reset test counters */
void reset_counters() {
    test_counter_1 = 0;
    test_counter_2 = 0;
    test_arg_received = 0;
}

/* Test: start_daemon() adds daemon to list */
static void test_start_daemon_basic(void **state) {
    (void) state; /* unused */
    clear_daemon_list();
    reset_counters();

    start_daemon(test_daemon_func_1, 42, BEFORE);

    /* Check that daemon is in list */
    int found = 0;
    for (int i = 0; i < MAXDAEMONS; i++) {
        if (d_list[i].d_func == test_daemon_func_1) {
            found = 1;
            assert_int_equal(BEFORE, d_list[i].d_type);
            assert_int_equal(42, d_list[i].d_arg);
            assert_int_equal(DAEMON, d_list[i].d_time);
            break;
        }
    }
    assert_true(found);
}

/* Test: do_daemons() executes matching daemons */
static void test_do_daemons_execution(void **state) {
    (void) state; /* unused */
    clear_daemon_list();
    reset_counters();

    start_daemon(test_daemon_func_1, 100, BEFORE);
    start_daemon(test_daemon_func_2, 200, AFTER);

    /* Execute BEFORE daemons */
    do_daemons(BEFORE);

    assert_int_equal(1, test_counter_1);
    assert_int_equal(100, test_arg_received);
    assert_int_equal(0, test_counter_2);  /* AFTER daemon not called */

    /* Execute AFTER daemons */
    do_daemons(AFTER);

    assert_int_equal(1, test_counter_1);  /* Still 1 */
    assert_int_equal(1, test_counter_2);  /* Now called */
}

/* Test: do_daemons() runs multiple times */
static void test_do_daemons_multiple_runs(void **state) {
    (void) state; /* unused */
    clear_daemon_list();
    reset_counters();

    start_daemon(test_daemon_func_1, 50, BEFORE);

    do_daemons(BEFORE);
    assert_int_equal(1, test_counter_1);

    do_daemons(BEFORE);
    assert_int_equal(2, test_counter_1);

    do_daemons(BEFORE);
    assert_int_equal(3, test_counter_1);
}

/* Test: kill_daemon() removes daemon */
static void test_kill_daemon_basic(void **state) {
    (void) state; /* unused */
    clear_daemon_list();
    reset_counters();

    start_daemon(test_daemon_func_1, 42, BEFORE);
    kill_daemon(test_daemon_func_1);

    /* Daemon should not execute after being killed */
    do_daemons(BEFORE);
    assert_int_equal(0, test_counter_1);

    /* Verify it's removed from list */
    int found = 0;
    for (int i = 0; i < MAXDAEMONS; i++) {
        if (d_list[i].d_func == test_daemon_func_1 &&
            d_list[i].d_type != EMPTY) {
            found = 1;
            break;
        }
    }
    assert_false(found);
}

/* Test: fuse() creates timed event */
static void test_fuse_basic(void **state) {
    (void) state; /* unused */
    clear_daemon_list();
    reset_counters();

    fuse(test_fuse_func, 99, 5, BEFORE);

    /* Check that fuse is in list */
    int found = 0;
    for (int i = 0; i < MAXDAEMONS; i++) {
        if (d_list[i].d_func == test_fuse_func) {
            found = 1;
            assert_int_equal(BEFORE, d_list[i].d_type);
            assert_int_equal(99, d_list[i].d_arg);
            assert_int_equal(5, d_list[i].d_time);
            break;
        }
    }
    assert_true(found);
}

/* Test: do_fuses() counts down and triggers */
static void test_do_fuses_countdown(void **state) {
    (void) state; /* unused */
    clear_daemon_list();
    reset_counters();

    fuse(test_fuse_func, 123, 3, BEFORE);

    /* First tick - time goes to 2 */
    do_fuses(BEFORE);
    assert_int_equal(0, test_counter_1);  /* Not triggered yet */

    /* Second tick - time goes to 1 */
    do_fuses(BEFORE);
    assert_int_equal(0, test_counter_1);  /* Still not triggered */

    /* Third tick - time goes to 0, fuse triggers */
    do_fuses(BEFORE);
    assert_int_equal(10, test_counter_1);  /* Fuse executed (adds 10) */
    assert_int_equal(123, test_arg_received);
}

/* Test: extinguish() removes fuse before it triggers */
static void test_extinguish_basic(void **state) {
    (void) state; /* unused */
    clear_daemon_list();
    reset_counters();

    fuse(test_fuse_func, 50, 5, BEFORE);

    /* Extinguish before it goes off */
    extinguish(test_fuse_func);

    /* Run fuses several times */
    for (int i = 0; i < 10; i++) {
        do_fuses(BEFORE);
    }

    /* Fuse should never have executed */
    assert_int_equal(0, test_counter_1);
}

/* Test: lengthen() extends fuse time */
static void test_lengthen_basic(void **state) {
    (void) state; /* unused */
    clear_daemon_list();
    reset_counters();

    fuse(test_fuse_func, 77, 3, BEFORE);

    /* Tick twice */
    do_fuses(BEFORE);
    do_fuses(BEFORE);

    /* Now at time=1, lengthen by 2 */
    lengthen(test_fuse_func, 2);

    /* Should now take 3 more ticks */
    do_fuses(BEFORE);
    assert_int_equal(0, test_counter_1);

    do_fuses(BEFORE);
    assert_int_equal(0, test_counter_1);

    do_fuses(BEFORE);
    assert_int_equal(10, test_counter_1);  /* Finally triggered */
}

/* Test: multiple daemons can coexist */
static void test_multiple_daemons(void **state) {
    (void) state; /* unused */
    clear_daemon_list();
    reset_counters();

    start_daemon(test_daemon_func_1, 1, BEFORE);
    start_daemon(test_daemon_func_2, 2, BEFORE);

    do_daemons(BEFORE);

    assert_int_equal(1, test_counter_1);
    assert_int_equal(1, test_counter_2);
}

/* Test: multiple fuses can coexist */
static void test_multiple_fuses(void **state) {
    (void) state; /* unused */
    clear_daemon_list();
    reset_counters();

    fuse(test_daemon_func_1, 10, 2, BEFORE);
    fuse(test_daemon_func_2, 20, 3, BEFORE);

    /* First fuse triggers at time 2 */
    do_fuses(BEFORE);
    assert_int_equal(0, test_counter_1);
    assert_int_equal(0, test_counter_2);

    do_fuses(BEFORE);
    assert_int_equal(1, test_counter_1);  /* First fuse triggered */
    assert_int_equal(0, test_counter_2);

    do_fuses(BEFORE);
    assert_int_equal(1, test_counter_1);
    assert_int_equal(1, test_counter_2);  /* Second fuse triggered */
}

/* Test: daemon and fuse can coexist */
static void test_daemon_and_fuse(void **state) {
    (void) state; /* unused */
    clear_daemon_list();
    reset_counters();

    start_daemon(test_daemon_func_1, 5, BEFORE);
    fuse(test_daemon_func_2, 10, 2, BEFORE);

    /* Daemon runs every time, fuse waits */
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    assert_int_equal(1, test_counter_1);
    assert_int_equal(0, test_counter_2);

    do_daemons(BEFORE);
    do_fuses(BEFORE);
    assert_int_equal(2, test_counter_1);
    assert_int_equal(1, test_counter_2);  /* Fuse now triggered */

    do_daemons(BEFORE);
    assert_int_equal(3, test_counter_1);
    assert_int_equal(1, test_counter_2);  /* Fuse doesn't repeat */
}

int run_daemon_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_start_daemon_basic),
        cmocka_unit_test(test_do_daemons_execution),
        cmocka_unit_test(test_do_daemons_multiple_runs),
        cmocka_unit_test(test_kill_daemon_basic),
        cmocka_unit_test(test_fuse_basic),
        cmocka_unit_test(test_do_fuses_countdown),
        cmocka_unit_test(test_extinguish_basic),
        cmocka_unit_test(test_lengthen_basic),
        cmocka_unit_test(test_multiple_daemons),
        cmocka_unit_test(test_multiple_fuses),
        cmocka_unit_test(test_daemon_and_fuse),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
