#include "test_framework.h"
#include "memory_helpers.h"
#include "../include/openpgp.h"
#include "../generated/bridge_builder.h"
#include "../generated/bridge_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void test_builder_create_destroy_no_leak(void) {
    // EXPECT: This should fail initially due to memory leaks
    // Create builder
    flatbuffers_builder_t *builder = flatbuffers_builder_create(1024);
    TEST_ASSERT_NOT_NULL_MESSAGE(builder, "Failed to create FlatBuffer builder");
    
    // Destroy builder immediately
    flatbuffers_builder_destroy(builder);
    
    // Memory tracking should show no leaks
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "FlatBuffer builder lifecycle leaked memory");
}

void test_builder_with_buffer_no_leak(void) {
    // EXPECT: This should fail initially due to memory leaks
    // Create builder
    flatbuffers_builder_t *builder = flatbuffers_builder_create(1024);
    TEST_ASSERT_NOT_NULL_MESSAGE(builder, "Failed to create FlatBuffer builder");
    
    // Get buffer (but don't use it)
    size_t size;
    uint8_t *buffer = flatbuffers_builder_get_direct_buffer(builder, &size);
    TEST_ASSERT_NOT_NULL_MESSAGE(buffer, "Failed to get buffer from builder");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, size, "Buffer size should be greater than 0");
    
    // Destroy builder
    flatbuffers_builder_destroy(builder);
    
    // Memory tracking should show no leaks
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "FlatBuffer builder with buffer leaked memory");
}

void test_multiple_builders_no_interference(void) {
    // EXPECT: This should fail initially due to memory leaks or interference
    // Create multiple builders
    flatbuffers_builder_t *builder1 = flatbuffers_builder_create(512);
    flatbuffers_builder_t *builder2 = flatbuffers_builder_create(1024);
    
    TEST_ASSERT_NOT_NULL(builder1);
    TEST_ASSERT_NOT_NULL(builder2);
    TEST_ASSERT_NOT_EQUAL(builder1, builder2);
    
    // Use both builders
    size_t size1, size2;
    uint8_t *buffer1 = flatbuffers_builder_get_direct_buffer(builder1, &size1);
    uint8_t *buffer2 = flatbuffers_builder_get_direct_buffer(builder2, &size2);
    
    TEST_ASSERT_NOT_NULL(buffer1);
    TEST_ASSERT_NOT_NULL(buffer2);
    TEST_ASSERT_NOT_EQUAL(buffer1, buffer2);
    
    // Destroy builders
    flatbuffers_builder_destroy(builder1);
    flatbuffers_builder_destroy(builder2);
    
    // Memory tracking should show no leaks
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "Multiple FlatBuffer builders leaked memory");
}

// Task #3 serialize_generate_request Buffer Management Tests (TDD - should fail initially)

void test_serialize_generate_minimal_request(void) {
    // EXPECT: This should fail initially due to buffer overruns
    openpgp_options_t options = {0};
    options.name = "";
    options.email = "";
    options.comment = "";
    options.passphrase = "";
    
    openpgp_key_options_t key_options = {0};
    key_options.algorithm = OPENPGP_ALGORITHM_RSA;
    key_options.rsa_bits = 2048;
    
    size_t buffer_size;
    uint8_t *buffer = NULL;
    
    // This function doesn't exist yet - we need to create it
    // For now, just test that we can create a minimal request without crashing
    flatbuffers_builder_t *builder = flatbuffers_builder_create(1024);
    TEST_ASSERT_NOT_NULL(builder);
    
    // Try to build minimal request
    // TODO: Implement serialize_generate_request function
    
    flatbuffers_builder_destroy(builder);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "Minimal generate request serialization leaked memory");
}

void test_serialize_generate_max_size_request(void) {
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
    
    openpgp_key_options_t key_options = {0};
    key_options.algorithm = OPENPGP_ALGORITHM_RSA;
    key_options.rsa_bits = 4096;  // Larger key size
    
    // Test that we can handle large inputs without buffer overruns
    flatbuffers_builder_t *builder = flatbuffers_builder_create(4096);  // Larger buffer
    TEST_ASSERT_NOT_NULL(builder);
    
    // TODO: Implement serialize_generate_request function
    
    flatbuffers_builder_destroy(builder);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "Large generate request serialization leaked memory");
}

// Main test runner
int main(void) {
    UNITY_BEGIN();
    
    printf("=== FlatBuffer Memory Test Suite ===\n");
    printf("Running tests with memory tracking enabled\n\n");
    
    // Infrastructure tests
    RUN_TEST(test_memory_tracking_basic_functionality);
    RUN_TEST(test_valgrind_availability);
    
    // Basic FlatBuffer lifecycle tests (expected to fail initially)
    printf("\n--- Basic FlatBuffer Lifecycle Tests ---\n");
    RUN_TEST(test_builder_create_destroy_no_leak);
    RUN_TEST(test_builder_with_buffer_no_leak);
    RUN_TEST(test_multiple_builders_no_interference);
    
    // Buffer management tests (expected to fail initially)
    printf("\n--- Buffer Management Tests ---\n");
    RUN_TEST(test_serialize_generate_minimal_request);
    RUN_TEST(test_serialize_generate_max_size_request);
    
    printf("\n=== Test Suite Complete ===\n");
    
    return UNITY_END();
}