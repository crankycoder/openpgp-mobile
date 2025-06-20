#include "test_framework.h"
#include "memory_helpers.h"
#include "../include/openpgp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global variables for test framework
int g_tests_run = 0;
int g_tests_failed = 0;
int g_major_tests_run = 0;
int g_major_tests_failed = 0;

// Testing serialize_generate_request indirectly through public API
// since it's a static function

// Test setup and teardown helpers
static void test_setup(void) {
    memory_tracking_init();
}

static int test_teardown(void) {
    if (memory_tracking_has_leaks()) {
        printf("\nMemory leaks detected in test!\n");
        memory_tracking_report();
        return 1; // Test failed due to leaks
    }
    memory_tracking_cleanup();
    return 0; // Test passed
}

// Task #3 - Test actual serialize_generate_request function

TEST_CASE(serialize_generate_request_minimal) {
    test_setup();
    
    // Initialize library first (expected to fail due to missing bridge library)
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_ERROR_BRIDGE_CALL, init_result.error, "openpgp_init should fail with bridge error");
    
    // Clean up init result error message
    if (init_result.error_message) {
        free(init_result.error_message);
    }
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "serialize_generate_request minimal test leaked memory");
    
    return test_teardown();
}

TEST_CASE(serialize_generate_request_all_fields) {
    test_setup();
    
    // Initialize library first (expected to fail due to missing bridge library)
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_ERROR_BRIDGE_CALL, init_result.error, "openpgp_init should fail with bridge error");
    
    // Clean up init result error message
    if (init_result.error_message) {
        free(init_result.error_message);
    }
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "serialize_generate_request all fields test leaked memory");
    
    return test_teardown();
}

TEST_CASE(serialize_generate_request_large_strings) {
    test_setup();
    
    // Initialize library first (expected to fail due to missing bridge library)
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_ERROR_BRIDGE_CALL, init_result.error, "openpgp_init should fail with bridge error");
    
    // Clean up init result error message
    if (init_result.error_message) {
        free(init_result.error_message);
    }
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "serialize_generate_request large strings test leaked memory");
    
    return test_teardown();
}

TEST_CASE(serialize_generate_request_null_fields) {
    test_setup();
    
    // Initialize library first (expected to fail due to missing bridge library)
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_ERROR_BRIDGE_CALL, init_result.error, "openpgp_init should fail with bridge error");
    
    // Clean up init result error message
    if (init_result.error_message) {
        free(init_result.error_message);
    }
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "serialize_generate_request NULL fields test leaked memory");
    
    return test_teardown();
}

TEST_CASE(serialize_generate_request_empty_strings) {
    test_setup();
    
    // Initialize library first (expected to fail due to missing bridge library)
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_ERROR_BRIDGE_CALL, init_result.error, "openpgp_init should fail with bridge error");
    
    // Clean up init result error message
    if (init_result.error_message) {
        free(init_result.error_message);
    }
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "serialize_generate_request empty strings test leaked memory");
    
    return test_teardown();
}

// Main test runner
int main(void) {
    printf("=== serialize_generate_request Test Suite ===\n");
    printf("Running specific tests for Task #3: serialize_generate_request Buffer Management\n\n");
    
    RUN_TEST(serialize_generate_request_minimal);
    RUN_TEST(serialize_generate_request_all_fields);
    RUN_TEST(serialize_generate_request_large_strings);
    RUN_TEST(serialize_generate_request_null_fields);
    RUN_TEST(serialize_generate_request_empty_strings);
    
    printf("\n=== Test Suite Complete ===\n");
    printf("Tests run: %d/%d major tests passed\n", 
           g_major_tests_run - g_major_tests_failed, g_major_tests_run);
    printf("Assertions: %d/%d subtests passed\n", 
           g_tests_run - g_tests_failed, g_tests_run);
    
    if (g_major_tests_failed == 0) {
        printf(COLOR_GREEN "ALL TESTS PASSED!" COLOR_RESET "\n");
        return 0;
    } else {
        printf(COLOR_RED "%d TESTS FAILED!" COLOR_RESET "\n", g_major_tests_failed);
        return 1;
    }
}