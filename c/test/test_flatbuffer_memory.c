#include "test_framework.h"
#include "memory_helpers.h"
#include "../include/openpgp.h"
#include "../generated/bridge_builder.h"
#include "../generated/bridge_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the actual FlatBuffer headers
#include "../generated/flatcc/flatcc_builder.h"

// Global variables for test framework
int g_tests_run = 0;
int g_tests_failed = 0;
int g_major_tests_run = 0;
int g_major_tests_failed = 0;

// Additional macros needed for this test
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

#define TEST_ASSERT_GREATER_THAN(lower, actual) \
    do { \
        g_tests_run++; \
        if ((actual) <= (lower)) { \
            g_tests_failed++; \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: Expected %d > %d\n", \
                   __FILE__, __LINE__, (int)(actual), (int)(lower)); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL_MESSAGE(ptr, msg) \
    do { \
        g_tests_run++; \
        if ((ptr) == NULL) { \
            g_tests_failed++; \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: %s\n", \
                   __FILE__, __LINE__, msg); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_FALSE_MESSAGE(condition, msg) \
    do { \
        g_tests_run++; \
        if (condition) { \
            g_tests_failed++; \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: %s\n", \
                   __FILE__, __LINE__, msg); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_GREATER_THAN_MESSAGE(lower, actual, msg) \
    do { \
        g_tests_run++; \
        if ((actual) <= (lower)) { \
            g_tests_failed++; \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: %s\n", \
                   __FILE__, __LINE__, msg); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL_MESSAGE(expected, actual, msg) \
    do { \
        g_tests_run++; \
        if ((expected) != (actual)) { \
            g_tests_failed++; \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: %s (expected %d, got %d)\n", \
                   __FILE__, __LINE__, msg, (int)(expected), (int)(actual)); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_NOT_EQUAL_MESSAGE(expected, actual, msg) \
    do { \
        g_tests_run++; \
        if ((expected) == (actual)) { \
            g_tests_failed++; \
            printf(COLOR_RED "FAIL" COLOR_RESET " %s:%d: %s (both values are %d)\n", \
                   __FILE__, __LINE__, msg, (int)(expected)); \
            return 1; \
        } \
    } while (0)

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

// Task #1 Infrastructure Tests

TEST_CASE(memory_tracking_basic_functionality) {
    test_setup();
    
    // Test that memory tracking itself doesn't leak
    size_t initial_count = memory_tracking_get_allocation_count();
    
    void* ptr = TRACKED_MALLOC(100);
    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_EQUAL(initial_count + 1, memory_tracking_get_allocation_count());
    TEST_ASSERT_EQUAL(100, memory_tracking_get_allocated_bytes());
    
    TRACKED_FREE(ptr);
    TEST_ASSERT_EQUAL(initial_count, memory_tracking_get_allocation_count());
    TEST_ASSERT_EQUAL(0, memory_tracking_get_allocated_bytes());
    
    return test_teardown();
}

TEST_CASE(valgrind_availability) {
    test_setup();
    
    // Test that valgrind detection works
    bool available = is_valgrind_available();
    printf("Valgrind available: %s\n", available ? "yes" : "no");
    
    // This test always passes, just reports availability
    TEST_ASSERT_TRUE(true);
    
    return test_teardown();
}

// Task #2 FlatBuffer Builder Lifecycle Tests (TDD - these should fail initially)

TEST_CASE(builder_create_destroy_no_leak) {
    test_setup();
    
    // EXPECT: This should fail initially due to memory leaks
    // Create builder using the actual flatcc API
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    memset(B, 0, sizeof(flatcc_builder_t));
    int init_result = flatcc_builder_init(B);
    TEST_ASSERT_EQUAL_MESSAGE(0, init_result, "Failed to initialize FlatBuffer builder");
    
    // Clear builder immediately
    flatcc_builder_clear(B);
    
    // Memory tracking should show no leaks
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "FlatBuffer builder lifecycle leaked memory");
    
    return test_teardown();
}

TEST_CASE(builder_with_buffer_no_leak) {
    test_setup();
    
    // EXPECT: This should fail initially due to memory leaks
    // Create builder using the actual flatcc API
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    memset(B, 0, sizeof(flatcc_builder_t));
    int init_result = flatcc_builder_init(B);
    TEST_ASSERT_EQUAL_MESSAGE(0, init_result, "Failed to initialize FlatBuffer builder");
    
    // Start a buffer
    int start_result = flatcc_builder_start_buffer(B, 0, 0, 0);
    TEST_ASSERT_EQUAL_MESSAGE(0, start_result, "Failed to start FlatBuffer");
    
    // Get buffer 
    size_t size;
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    TEST_ASSERT_NOT_NULL_MESSAGE(buffer, "Failed to get buffer from builder");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, size, "Buffer size should be greater than 0");
    
    // Clear builder
    flatcc_builder_clear(B);
    
    // Memory tracking should show no leaks
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "FlatBuffer builder with buffer leaked memory");
    
    return test_teardown();
}

TEST_CASE(multiple_builders_no_interference) {
    test_setup();
    
    // EXPECT: This should fail initially due to memory leaks or interference
    // Create multiple builders using the actual flatcc API
    flatcc_builder_t builder1, builder2;
    flatcc_builder_t *B1 = &builder1;
    flatcc_builder_t *B2 = &builder2;
    
    memset(B1, 0, sizeof(flatcc_builder_t));
    memset(B2, 0, sizeof(flatcc_builder_t));
    
    int init_result1 = flatcc_builder_init(B1);
    int init_result2 = flatcc_builder_init(B2);
    
    TEST_ASSERT_EQUAL_MESSAGE(0, init_result1, "Failed to initialize first FlatBuffer builder");
    TEST_ASSERT_EQUAL_MESSAGE(0, init_result2, "Failed to initialize second FlatBuffer builder");
    TEST_ASSERT_NOT_EQUAL(B1, B2);
    
    // Use both builders
    int start_result1 = flatcc_builder_start_buffer(B1, 0, 0, 0);
    int start_result2 = flatcc_builder_start_buffer(B2, 0, 0, 0);
    
    TEST_ASSERT_EQUAL_MESSAGE(0, start_result1, "Failed to start first buffer");
    TEST_ASSERT_EQUAL_MESSAGE(0, start_result2, "Failed to start second buffer");
    
    size_t size1, size2;
    void *buffer1 = flatcc_builder_get_direct_buffer(B1, &size1);
    void *buffer2 = flatcc_builder_get_direct_buffer(B2, &size2);
    
    TEST_ASSERT_NOT_NULL(buffer1);
    TEST_ASSERT_NOT_NULL(buffer2);
    TEST_ASSERT_NOT_EQUAL(buffer1, buffer2);
    
    // Clear builders
    flatcc_builder_clear(B1);
    flatcc_builder_clear(B2);
    
    // Memory tracking should show no leaks
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "Multiple FlatBuffer builders leaked memory");
    
    return test_teardown();
}

// Task #3 serialize_generate_request Buffer Management Tests (TDD - should fail initially)

TEST_CASE(serialize_generate_minimal_request) {
    test_setup();
    
    // EXPECT: This should fail initially due to buffer overruns
    openpgp_options_t options = {0};
    options.name = "";
    options.email = "";
    options.comment = "";
    options.passphrase = "";
    
    // Test that we can create a minimal request without crashing
    // This mimics the actual serialize_generate_request function pattern
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    memset(B, 0, sizeof(flatcc_builder_t));
    int init_result = flatcc_builder_init(B);
    TEST_ASSERT_EQUAL_MESSAGE(0, init_result, "Failed to initialize FlatBuffer builder");
    
    int start_result = flatcc_builder_start_buffer(B, 0, 0, 0);
    TEST_ASSERT_EQUAL_MESSAGE(0, start_result, "Failed to start FlatBuffer");
    
    // Try to create minimal strings (this is where buffer overruns might occur)
    flatbuffers_string_ref_t name_ref = flatbuffers_string_create_str(B, options.name ? options.name : "");
    flatbuffers_string_ref_t email_ref = flatbuffers_string_create_str(B, options.email ? options.email : "");
    
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, name_ref, "Failed to create name string reference");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, email_ref, "Failed to create email string reference");
    
    // Clear builder 
    flatcc_builder_clear(B);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "Minimal generate request serialization leaked memory");
    
    return test_teardown();
}

TEST_CASE(serialize_generate_max_size_request) {
    test_setup();
    
    // EXPECT: This should fail initially due to buffer sizing issues
    openpgp_options_t options = {0};
    
    // Create maximum size strings (but reasonable for testing)
    char large_name[256];
    char large_email[256];  
    char large_comment[512];
    char large_passphrase[128];
    
    memset(large_name, 'A', sizeof(large_name) - 1);
    large_name[sizeof(large_name) - 1] = '\0';
    
    memset(large_email, 'B', sizeof(large_email) - 1);
    large_email[sizeof(large_email) - 1] = '\0';
    
    memset(large_comment, 'C', sizeof(large_comment) - 1);
    large_comment[sizeof(large_comment) - 1] = '\0';
    
    memset(large_passphrase, 'D', sizeof(large_passphrase) - 1);
    large_passphrase[sizeof(large_passphrase) - 1] = '\0';
    
    options.name = large_name;
    options.email = large_email;
    options.comment = large_comment;
    options.passphrase = large_passphrase;
    
    // Test that we can handle large inputs without buffer overruns
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    memset(B, 0, sizeof(flatcc_builder_t));
    int init_result = flatcc_builder_init(B);
    TEST_ASSERT_EQUAL_MESSAGE(0, init_result, "Failed to initialize FlatBuffer builder");
    
    int start_result = flatcc_builder_start_buffer(B, 0, 0, 0);
    TEST_ASSERT_EQUAL_MESSAGE(0, start_result, "Failed to start FlatBuffer");
    
    // Try to create large strings (this is where buffer overruns might occur)
    flatbuffers_string_ref_t name_ref = flatbuffers_string_create_str(B, options.name);
    flatbuffers_string_ref_t email_ref = flatbuffers_string_create_str(B, options.email);
    flatbuffers_string_ref_t comment_ref = flatbuffers_string_create_str(B, options.comment);
    flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(B, options.passphrase);
    
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, name_ref, "Failed to create large name string reference");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, email_ref, "Failed to create large email string reference");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, comment_ref, "Failed to create large comment string reference");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, passphrase_ref, "Failed to create large passphrase string reference");
    
    // Clear builder
    flatcc_builder_clear(B);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "Large generate request serialization leaked memory");
    
    return test_teardown();
}

// Main test runner
int main(void) {
    printf("=== FlatBuffer Memory Test Suite ===\n");
    printf("Running tests with memory tracking enabled\n\n");
    
    // Infrastructure tests
    RUN_TEST(memory_tracking_basic_functionality);
    RUN_TEST(valgrind_availability);
    
    // Basic FlatBuffer lifecycle tests (expected to fail initially)
    printf("\n--- Basic FlatBuffer Lifecycle Tests ---\n");
    RUN_TEST(builder_create_destroy_no_leak);
    RUN_TEST(builder_with_buffer_no_leak);
    RUN_TEST(multiple_builders_no_interference);
    
    // Buffer management tests (expected to fail initially)
    printf("\n--- Buffer Management Tests ---\n");
    RUN_TEST(serialize_generate_minimal_request);
    RUN_TEST(serialize_generate_max_size_request);
    
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