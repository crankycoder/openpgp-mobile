#ifndef TEST_FRAMEWORK_ISOLATED_H
#define TEST_FRAMEWORK_ISOLATED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_isolation_manager.h"

/* Colors for output */
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"

/* Isolated test macros that use the isolation manager */
#define TEST_ASSERT(condition) \
    do { \
        increment_tests_run(); \
        if (!(condition)) { \
            increment_tests_failed(); \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Assertion failed: %s\n", \
                   __FILE__, __LINE__, #condition); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        increment_tests_run(); \
        if ((expected) != (actual)) { \
            increment_tests_failed(); \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected %d, got %d\n", \
                   __FILE__, __LINE__, (int)(expected), (int)(actual)); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        increment_tests_run(); \
        if ((ptr) == NULL) { \
            increment_tests_failed(); \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected non-NULL pointer\n", \
                   __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_NULL(ptr) \
    do { \
        increment_tests_run(); \
        if ((ptr) != NULL) { \
            increment_tests_failed(); \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected NULL pointer\n", \
                   __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_STRING_EQUAL(expected, actual) \
    do { \
        increment_tests_run(); \
        if (!expected || !actual || strcmp(expected, actual) != 0) { \
            increment_tests_failed(); \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected \"%s\", got \"%s\"\n", \
                   __FILE__, __LINE__, expected ? expected : "NULL", actual ? actual : "NULL"); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_TRUE(condition) \
    do { \
        increment_tests_run(); \
        if (!(condition)) { \
            increment_tests_failed(); \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected true, got false: %s\n", \
                   __FILE__, __LINE__, #condition); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_FALSE(condition) \
    do { \
        increment_tests_run(); \
        if (condition) { \
            increment_tests_failed(); \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected false, got true: %s\n", \
                   __FILE__, __LINE__, #condition); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_NOT_EQUAL(expected, actual) \
    do { \
        increment_tests_run(); \
        if ((expected) == (actual)) { \
            increment_tests_failed(); \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected not %d, but got %d\n", \
                   __FILE__, __LINE__, (int)(expected), (int)(actual)); \
            return 1; \
        } \
    } while (0)/* Test execution macros for isolated tests */
#define RUN_ISOLATED_TEST(name) \
    run_isolated_test(test_##name, #name)

/* Test control flow macros */
#define TEST_PASS() \
    do { \
        printf(COLOR_GREEN "OK" COLOR_RESET "\n"); \
        return 0; \
    } while (0)

#define TEST_FAIL() \
    do { \
        printf(COLOR_RED "FAILED" COLOR_RESET "\n"); \
        return 1; \
    } while (0)

#define TEST_LOG(...) \
    do { \
        printf("    "); \
        printf(__VA_ARGS__); \
        printf("\n"); \
    } while (0)

/* Message versions of test macros */
#define TEST_ASSERT_EQUAL_MESSAGE(expected, actual, msg) \
    do { \
        increment_tests_run(); \
        if ((expected) != (actual)) { \
            increment_tests_failed(); \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: %s (Expected %d, got %d)\n", \
                   __FILE__, __LINE__, msg, (int)(expected), (int)(actual)); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL_MESSAGE(ptr, msg) \
    do { \
        increment_tests_run(); \
        if ((ptr) == NULL) { \
            increment_tests_failed(); \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: %s (Expected non-NULL pointer)\n", \
                   __FILE__, __LINE__, msg); \
            return 1; \
        } \
    } while (0)

#endif /* TEST_FRAMEWORK_ISOLATED_H */