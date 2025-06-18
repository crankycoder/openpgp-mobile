#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test result tracking */
extern int g_tests_run;
extern int g_tests_failed;

/* Colors for output */
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"

/* Test macros */
#define TEST_ASSERT(condition) \
    do { \
        g_tests_run++; \
        if (!(condition)) { \
            g_tests_failed++; \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Assertion failed: %s\n", \
                   __FILE__, __LINE__, #condition); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        g_tests_run++; \
        if ((expected) != (actual)) { \
            g_tests_failed++; \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected %d, got %d\n", \
                   __FILE__, __LINE__, (int)(expected), (int)(actual)); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        g_tests_run++; \
        if ((ptr) == NULL) { \
            g_tests_failed++; \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected non-NULL pointer\n", \
                   __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_NULL(ptr) \
    do { \
        g_tests_run++; \
        if ((ptr) != NULL) { \
            g_tests_failed++; \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected NULL pointer\n", \
                   __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_STRING_EQUAL(expected, actual) \
    do { \
        g_tests_run++; \
        if (!expected || !actual || strcmp(expected, actual) != 0) { \
            g_tests_failed++; \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected \"%s\", got \"%s\"\n", \
                   __FILE__, __LINE__, expected ? expected : "NULL", actual ? actual : "NULL"); \
            return 1; \
        } \
    } while (0)

/* Test case declaration */
#define TEST_CASE(name) \
    int test_##name(void); \
    int test_##name(void)

/* Test runner macros */
#define RUN_TEST(name) \
    do { \
        printf("Running test_" #name "... "); \
        if (test_##name() == 0) { \
            printf(COLOR_GREEN "PASS" COLOR_RESET "\n"); \
        } else { \
            printf(COLOR_RED "FAIL" COLOR_RESET "\n"); \
        } \
    } while (0)

#endif /* TEST_FRAMEWORK_H */