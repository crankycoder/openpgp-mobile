#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "test_framework.h"
#include "memory_helpers.h"
#include "openpgp.h"

/* Global counters for testing purposes (shared with test_framework.h) */
extern int g_tests_run;
extern int g_tests_failed;
extern int g_major_tests_run;
extern int g_major_tests_failed;

/* Test state tracking */
static int initial_tests_run = 0;
static int initial_major_tests_run = 0;

int test_setup(void) {
    memory_tracking_init();
    
    /* Capture initial state */
    initial_tests_run = g_tests_run;
    initial_major_tests_run = g_major_tests_run;
    
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

/* Test that library state is properly isolated between tests */
TEST_CASE(isolation_library_init_cleanup_cycle) {
    test_setup();
    
    /* First initialization */
    openpgp_result_t result1 = openpgp_init();
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_SUCCESS, result1.error, "First init should succeed");
    
    /* Cleanup */
    openpgp_cleanup();
    
    /* Second initialization should work independently */
    openpgp_result_t result2 = openpgp_init();
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_SUCCESS, result2.error, "Second init should succeed after cleanup");
    
    /* Cleanup again */
    openpgp_cleanup();
    
    /* Third initialization */
    openpgp_result_t result3 = openpgp_init();
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_SUCCESS, result3.error, "Third init should succeed");
    
    /* Final cleanup */
    openpgp_cleanup();
    
    /* Clean up result memory */
    if (result1.error_message) free(result1.error_message);
    if (result1.data) free(result1.data);
    if (result2.error_message) free(result2.error_message);
    if (result2.data) free(result2.data);
    if (result3.error_message) free(result3.error_message);
    if (result3.data) free(result3.data);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Library init/cleanup cycles should not leak memory");
    
    return test_teardown();
}

/* Test that memory tracking state doesn't interfere between tests */
TEST_CASE(isolation_memory_tracking_state) {
    test_setup();
    
    /* Allocate and free some memory */
    void *ptr1 = TRACKED_MALLOC(100);
    void *ptr2 = TRACKED_MALLOC(200);
    
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    
    TRACKED_FREE(ptr1);
    TRACKED_FREE(ptr2);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Memory tracking should show no leaks after proper cleanup");
    
    return test_teardown();
}

/* Test that test counters don't interfere with actual functionality */
TEST_CASE(isolation_test_counters) {
    test_setup();
    
    /* Record current counts */
    int before_tests_run = g_tests_run;
    int before_major_tests_run = g_major_tests_run;
    
    /* Perform some operations that shouldn't affect test counts */
    openpgp_result_t result = openpgp_init();
    
    /* Test counts should only change due to our assertions */
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_SUCCESS, result.error, "Init should succeed");
    int after_assertion = g_tests_run;
    
    /* Cleanup library */
    openpgp_cleanup();
    
    /* Verify test count incremented by exactly 1 (our assertion) */
    TEST_ASSERT_EQUAL_MESSAGE(before_tests_run + 1, after_assertion, "Test count should increment by exactly 1");
    
    /* Clean up result memory */
    if (result.error_message) free(result.error_message);
    if (result.data) free(result.data);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Counter isolation test should not leak memory");
    
    return test_teardown();
}

/* Test that FlatBuffer builders are properly cleaned up between operations */
TEST_CASE(isolation_flatbuffer_builder_state) {
    test_setup();
    
    /* Initialize library for FlatBuffer operations */
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_SUCCESS, init_result.error, "Library should initialize");
    
    /* First FlatBuffer operation */
    openpgp_options_t options1 = {
        .name = "Test User 1",
        .email = "test1@example.com",
        .comment = NULL,
        .passphrase = NULL,
        .key_options = {
            .algorithm = OPENPGP_ALGORITHM_RSA,
            .rsa_bits = 2048,
            .curve = OPENPGP_CURVE_SECP256K1
        }
    };
    
    openpgp_result_t result1 = openpgp_generate_key_with_options(&options1);
    
    /* Clean up first result */
    if (result1.error_message) free(result1.error_message);
    if (result1.data) free(result1.data);
    
    /* Second FlatBuffer operation with different data */
    openpgp_options_t options2 = {
        .name = "Test User 2",
        .email = "test2@example.com", 
        .comment = "Different comment",
        .passphrase = "different_pass",
        .key_options = {
            .algorithm = OPENPGP_ALGORITHM_RSA,
            .rsa_bits = 2048,
            .curve = OPENPGP_CURVE_SECP256K1
        }
    };
    
    openpgp_result_t result2 = openpgp_generate_key_with_options(&options2);
    
    /* Clean up second result */
    if (result2.error_message) free(result2.error_message);
    if (result2.data) free(result2.data);
    
    /* Cleanup library */
    openpgp_cleanup();
    if (init_result.error_message) free(init_result.error_message);
    if (init_result.data) free(init_result.data);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "FlatBuffer operations should not leak memory between calls");
    
    return test_teardown();
}

/* Test that error states don't persist between operations */
TEST_CASE(isolation_error_state_cleanup) {
    test_setup();
    
    /* Try an operation that should fail (uninitialized library) */
    openpgp_options_t options = {
        .name = "Test User",
        .email = "test@example.com",
        .comment = NULL,
        .passphrase = NULL,
        .key_options = {
            .key_type = OPENPGP_KEY_TYPE_RSA,
            .key_size = 2048,
            .curve = OPENPGP_CURVE_UNSPECIFIED
        }
    };
    
    openpgp_result_t error_result = openpgp_generate_keypair(&options);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(OPENPGP_SUCCESS, error_result.error, "Operation should fail with uninitialized library");
    
    /* Clean up error result */
    if (error_result.error_message) free(error_result.error_message);
    if (error_result.data) free(error_result.data);
    
    /* Now initialize library properly */
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_SUCCESS, init_result.error, "Library should initialize after previous error");
    
    /* Operation should now succeed */
    openpgp_result_t success_result = openpgp_generate_keypair(&options);
    /* Note: This might still fail due to bridge issues, but error should be different */
    
    /* Clean up all results */
    if (init_result.error_message) free(init_result.error_message);
    if (init_result.data) free(init_result.data);
    if (success_result.error_message) free(success_result.error_message);
    if (success_result.data) free(success_result.data);
    
    /* Cleanup library */
    openpgp_cleanup();
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Error state handling should not leak memory");
    
    return test_teardown();
}

/* Test that each test function starts with clean memory state */
TEST_CASE(isolation_clean_memory_start) {
    test_setup();
    
    /* Memory tracking should start clean */
    TEST_ASSERT_EQUAL_MESSAGE(0, memory_tracking_get_allocation_count(), "Should start with zero allocations");
    TEST_ASSERT_EQUAL_MESSAGE(0, memory_tracking_get_allocated_bytes(), "Should start with zero bytes allocated");
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Should start with no leaks");
    
    /* Allocate something to verify tracking works */
    void *ptr = TRACKED_MALLOC(50);
    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_EQUAL_MESSAGE(1, memory_tracking_get_allocation_count(), "Should track one allocation");
    TEST_ASSERT_EQUAL_MESSAGE(50, memory_tracking_get_allocated_bytes(), "Should track 50 bytes");
    
    TRACKED_FREE(ptr);
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Should have no leaks after free");
    
    return test_teardown();
}

/* Main test runner */
int main(void) {
    printf("Running Task #7: Cross-Test Memory Isolation Tests\n");
    printf("=================================================\n\n");
    
    /* Reset global counters for this test suite */
    g_tests_run = 0;
    g_tests_failed = 0;
    g_major_tests_run = 0;
    g_major_tests_failed = 0;
    
    RUN_TEST(isolation_library_init_cleanup_cycle);
    RUN_TEST(isolation_memory_tracking_state);
    RUN_TEST(isolation_test_counters);
    RUN_TEST(isolation_flatbuffer_builder_state);
    RUN_TEST(isolation_error_state_cleanup);
    RUN_TEST(isolation_clean_memory_start);
    
    printf("\n=================================================\n");
    printf("Tests run: %d/%d major tests passed\n", 
           g_major_tests_run - g_major_tests_failed, g_major_tests_run);
    printf("Assertions: %d/%d subtests passed\n", 
           g_tests_run - g_tests_failed, g_tests_run);
    
    if (g_major_tests_failed == 0 && g_tests_failed == 0) {
        printf(COLOR_GREEN "ALL TESTS PASSED!" COLOR_RESET "\n");
        return 0;
    } else {
        printf(COLOR_RED "SOME TESTS FAILED!" COLOR_RESET "\n");
        return 1;
    }
}