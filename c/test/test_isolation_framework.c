#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "test_framework.h"
#include "memory_helpers.h"

/* Add missing TEST_ASSERT_FALSE macro */
#define TEST_ASSERT_FALSE(condition) \
    do { \
        g_tests_run++; \
        if (condition) { \
            g_tests_failed++; \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected false, got true: %s\n", \
                   __FILE__, __LINE__, #condition); \
            return 1; \
        } \
    } while (0)

/* Global test counters */
int g_tests_run = 0;
int g_tests_failed = 0;
int g_major_tests_run = 0;
int g_major_tests_failed = 0;

/* Test isolation framework - focused on memory and global state only */

int test_setup(void) {
    memory_tracking_init();
    return 0;
}

int test_teardown(void) {
    if (!memory_tracking_has_leaks()) {
        memory_tracking_cleanup();
        return 0;
    } else {
        memory_tracking_cleanup();
        return 1;
    }
}

/* Test that memory tracking properly resets between test functions */
TEST_CASE(isolation_memory_reset_between_tests) {
    test_setup();
    
    /* Memory tracking should start clean */
    TEST_ASSERT_EQUAL_MESSAGE(0, memory_tracking_get_allocation_count(), "Should start with zero allocations");
    TEST_ASSERT_EQUAL_MESSAGE(0, memory_tracking_get_allocated_bytes(), "Should start with zero bytes allocated");
    
    /* Allocate some memory */
    void *ptr1 = TRACKED_MALLOC(100);
    void *ptr2 = TRACKED_MALLOC(200);
    
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    TEST_ASSERT_EQUAL_MESSAGE(2, memory_tracking_get_allocation_count(), "Should track two allocations");
    TEST_ASSERT_EQUAL_MESSAGE(300, memory_tracking_get_allocated_bytes(), "Should track 300 bytes total");
    
    /* Free memory */
    TRACKED_FREE(ptr1);
    TRACKED_FREE(ptr2);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Should have no leaks after proper cleanup");
    
    return test_teardown();
}

/* Test that subsequent test starts with clean state */
TEST_CASE(isolation_clean_start_after_previous_test) {
    test_setup();
    
    /* This test should start completely clean, regardless of what previous test did */
    TEST_ASSERT_EQUAL_MESSAGE(0, memory_tracking_get_allocation_count(), "Should start clean after previous test");
    TEST_ASSERT_EQUAL_MESSAGE(0, memory_tracking_get_allocated_bytes(), "Should start with zero bytes after previous test");
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Should start with no leaks after previous test");
    
    /* Do some allocations to verify independence */
    void *ptr = TRACKED_MALLOC(500);
    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_EQUAL_MESSAGE(1, memory_tracking_get_allocation_count(), "Should track exactly one allocation");
    TEST_ASSERT_EQUAL_MESSAGE(500, memory_tracking_get_allocated_bytes(), "Should track exactly 500 bytes");
    
    TRACKED_FREE(ptr);
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Should clean up properly");
    
    return test_teardown();
}

/* Test that test counter state doesn't leak between tests */
TEST_CASE(isolation_test_counter_independence) {
    test_setup();
    
    /* Record the counter at start of this test */
    int start_count = g_tests_run;
    
    /* The counter should increment only due to our assertions */
    TEST_ASSERT_TRUE_MESSAGE(g_tests_run == start_count + 1, "Counter should increment by exactly 1 per assertion");
    
    int after_first_assertion = g_tests_run;
    
    /* Another assertion */
    TEST_ASSERT_EQUAL_MESSAGE(start_count + 2, after_first_assertion + 1, "Counter should continue incrementing predictably");
    
    /* Memory should still be clean */
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Counter operations should not affect memory tracking");
    
    return test_teardown();
}

/* Test that multiple allocation/free cycles work independently */
TEST_CASE(isolation_multiple_allocation_cycles) {
    test_setup();
    
    /* First cycle */
    void *ptr1 = TRACKED_MALLOC(100);
    TEST_ASSERT_NOT_NULL(ptr1);
    TRACKED_FREE(ptr1);
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "First cycle should be clean");
    
    /* Second cycle */
    void *ptr2 = TRACKED_MALLOC(200);
    TEST_ASSERT_NOT_NULL(ptr2);
    TRACKED_FREE(ptr2);
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Second cycle should be clean");
    
    /* Third cycle with multiple allocations */
    void *ptrA = TRACKED_MALLOC(50);
    void *ptrB = TRACKED_MALLOC(75);
    void *ptrC = TRACKED_MALLOC(125);
    
    TEST_ASSERT_NOT_NULL(ptrA);
    TEST_ASSERT_NOT_NULL(ptrB);
    TEST_ASSERT_NOT_NULL(ptrC);
    
    TEST_ASSERT_EQUAL_MESSAGE(3, memory_tracking_get_allocation_count(), "Should track 3 allocations in third cycle");
    TEST_ASSERT_EQUAL_MESSAGE(250, memory_tracking_get_allocated_bytes(), "Should track 250 bytes in third cycle");
    
    TRACKED_FREE(ptrA);
    TRACKED_FREE(ptrB);
    TRACKED_FREE(ptrC);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Third cycle should be clean");
    
    return test_teardown();
}

/* Test error handling isolation */
TEST_CASE(isolation_error_handling_state) {
    test_setup();
    
    /* Simulate an error condition by allocating without freeing */
    void *leaked_ptr = TRACKED_MALLOC(100);
    TEST_ASSERT_NOT_NULL(leaked_ptr);
    
    /* Verify the leak is detected */
    TEST_ASSERT_TRUE_MESSAGE(memory_tracking_has_leaks(), "Should detect the intentional leak");
    TEST_ASSERT_EQUAL_MESSAGE(1, memory_tracking_get_allocation_count(), "Should show one leaked allocation");
    
    /* Clean up the leak */
    TRACKED_FREE(leaked_ptr);
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Should be clean after fixing the leak");
    
    return test_teardown();
}

/* Test that string operations are properly isolated */
TEST_CASE(isolation_string_operations) {
    test_setup();
    
    /* String duplication operations */
    char *str1 = TRACKED_STRDUP("Test string 1");
    char *str2 = TRACKED_STRDUP("Test string 2 - longer");
    char *str3 = TRACKED_STRDUP("Short");
    
    TEST_ASSERT_NOT_NULL(str1);
    TEST_ASSERT_NOT_NULL(str2);
    TEST_ASSERT_NOT_NULL(str3);
    
    /* Verify string contents */
    TEST_ASSERT_STRING_EQUAL("Test string 1", str1);
    TEST_ASSERT_STRING_EQUAL("Test string 2 - longer", str2);
    TEST_ASSERT_STRING_EQUAL("Short", str3);
    
    /* Verify memory tracking */
    TEST_ASSERT_EQUAL_MESSAGE(3, memory_tracking_get_allocation_count(), "Should track 3 string allocations");
    
    /* Clean up strings */
    TRACKED_FREE(str1);
    TRACKED_FREE(str2);
    TRACKED_FREE(str3);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "String operations should not leak memory");
    
    return test_teardown();
}

/* Test that test framework macros work consistently across tests */
TEST_CASE(isolation_test_framework_consistency) {
    test_setup();
    
    /* Test basic assertions */
    TEST_ASSERT_TRUE(1 == 1);
    TEST_ASSERT_FALSE(1 == 0);
    TEST_ASSERT_EQUAL(42, 42);
    TEST_ASSERT_NOT_EQUAL(42, 43);
    
    /* Test NULL checks */
    void *null_ptr = NULL;
    void *non_null_ptr = TRACKED_MALLOC(1);
    
    TEST_ASSERT_NULL(null_ptr);
    TEST_ASSERT_NOT_NULL(non_null_ptr);
    
    TRACKED_FREE(non_null_ptr);
    
    /* Test string assertions */
    TEST_ASSERT_STRING_EQUAL("hello", "hello");
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Framework consistency test should not leak memory");
    
    return test_teardown();
}

/* Main test runner for isolation framework tests */
int main(void) {
    printf("Running Task #7: Cross-Test Memory Isolation Framework Tests\n");
    printf("============================================================\n\n");
    
    /* Reset global counters for this test suite */
    g_tests_run = 0;
    g_tests_failed = 0;
    g_major_tests_run = 0;
    g_major_tests_failed = 0;
    
    /* Run isolation tests in order */
    RUN_TEST(isolation_memory_reset_between_tests);
    RUN_TEST(isolation_clean_start_after_previous_test);
    RUN_TEST(isolation_test_counter_independence);
    RUN_TEST(isolation_multiple_allocation_cycles);
    RUN_TEST(isolation_error_handling_state);
    RUN_TEST(isolation_string_operations);
    RUN_TEST(isolation_test_framework_consistency);
    
    printf("\n============================================================\n");
    printf("Tests run: %d/%d major tests passed\n", 
           g_major_tests_run - g_major_tests_failed, g_major_tests_run);
    printf("Assertions: %d/%d subtests passed\n", 
           g_tests_run - g_tests_failed, g_tests_run);
    
    if (g_major_tests_failed == 0 && g_tests_failed == 0) {
        printf(COLOR_GREEN "ALL ISOLATION TESTS PASSED!" COLOR_RESET "\n");
        printf("✅ Memory tracking isolation is working correctly\n");
        printf("✅ Test framework isolation is working correctly\n");
        printf("✅ Cross-test interference is properly prevented\n");
        return 0;
    } else {
        printf(COLOR_RED "SOME ISOLATION TESTS FAILED!" COLOR_RESET "\n");
        printf("❌ Cross-test isolation issues detected\n");
        return 1;
    }
}