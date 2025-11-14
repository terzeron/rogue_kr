/*
 * Test Framework Header
 *
 * Common testing macros and utilities for all test files
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <limits.h>

/* Test statistics */
typedef struct {
    int tests_run;
    int tests_passed;
    int tests_failed;
    char *current_suite;
} TestStats;

extern TestStats global_stats;

/* Test framework macros */
#define TEST(name) void test_##name()

#define RUN_TEST(name) do { \
    printf("  %-50s", #name); \
    fflush(stdout); \
    global_stats.tests_run++; \
    test_##name(); \
    printf("PASSED\n"); \
    global_stats.tests_passed++; \
} while(0)

#define ASSERT_EQ(expected, actual) do { \
    if ((expected) != (actual)) { \
        fprintf(stderr, "\n  FAILED: %s == %s\n", #expected, #actual); \
        fprintf(stderr, "    Expected: %d\n", (int)(expected)); \
        fprintf(stderr, "    Actual: %d\n", (int)(actual)); \
        fprintf(stderr, "    Location: %s:%d\n", __FILE__, __LINE__); \
        global_stats.tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_TRUE(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "\n  FAILED: %s\n", #condition); \
        fprintf(stderr, "    Location: %s:%d\n", __FILE__, __LINE__); \
        global_stats.tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#define ASSERT_STR_EQ(expected, actual) do { \
    if (strcmp((expected), (actual)) != 0) { \
        fprintf(stderr, "\n  FAILED: strings not equal\n"); \
        fprintf(stderr, "    Expected: '%s'\n", (expected)); \
        fprintf(stderr, "    Actual: '%s'\n", (actual)); \
        fprintf(stderr, "    Location: %s:%d\n", __FILE__, __LINE__); \
        global_stats.tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_NOT_NULL(ptr) do { \
    if ((ptr) == NULL) { \
        fprintf(stderr, "\n  FAILED: %s is NULL\n", #ptr); \
        fprintf(stderr, "    Location: %s:%d\n", __FILE__, __LINE__); \
        global_stats.tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_NULL(ptr) do { \
    if ((ptr) != NULL) { \
        fprintf(stderr, "\n  FAILED: %s is not NULL\n", #ptr); \
        fprintf(stderr, "    Location: %s:%d\n", __FILE__, __LINE__); \
        global_stats.tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_IN_RANGE(value, min, max) do { \
    if ((value) < (min) || (value) > (max)) { \
        fprintf(stderr, "\n  FAILED: %s not in range [%d, %d]\n", #value, (int)(min), (int)(max)); \
        fprintf(stderr, "    Actual: %d\n", (int)(value)); \
        fprintf(stderr, "    Location: %s:%d\n", __FILE__, __LINE__); \
        global_stats.tests_failed++; \
        return; \
    } \
} while(0)

/* Test suite macros */
#define BEGIN_TEST_SUITE(name) \
    void run_##name##_tests() { \
        printf("\n=== Running %s Tests ===\n", #name); \
        global_stats.current_suite = #name;

#define END_TEST_SUITE() \
    }

/* Helper function declarations */
void init_test_stats(void);
void print_test_summary(void);

#endif /* TEST_FRAMEWORK_H */
