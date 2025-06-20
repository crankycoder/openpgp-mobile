#include "test_framework.h"
#include "memory_helpers.h"
#include "../include/openpgp.h"
#include "../generated/bridge_builder.h"
#include "../generated/bridge_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Global variables for test framework
int g_tests_run = 0;
int g_tests_failed = 0;
int g_major_tests_run = 0;
int g_major_tests_failed = 0;

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

// Helper function to create and test string serialization
static int test_string_creation(const char *test_string, const char *test_name) {
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    memset(B, 0, sizeof(flatcc_builder_t));
    if (flatcc_builder_init(B)) {
        printf("Failed to initialize FlatBuffer builder for %s\n", test_name);
        return 0;
    }
    
    if (flatcc_builder_start_buffer(B, 0, 0, 0)) {
        flatcc_builder_clear(B);
        printf("Failed to start FlatBuffer for %s\n", test_name);
        return 0;
    }
    
    // Test string creation
    flatbuffers_string_ref_t string_ref = 0;
    if (test_string) {
        string_ref = flatbuffers_string_create_str(B, test_string);
        if (string_ref == 0) {
            flatcc_builder_clear(B);
            printf("Failed to create string reference for %s\n", test_name);
            return 0;
        }
    }
    
    // Create a simple Options object to hold the string
    model_Options_start(B);
    if (string_ref) {
        model_Options_name_add(B, string_ref);
    }
    model_Options_ref_t options_ref = model_Options_end(B);
    
    // Create a GenerateRequest to finalize
    model_GenerateRequest_ref_t request_ref = model_GenerateRequest_create(B, options_ref);
    
    // End the buffer
    flatbuffers_buffer_ref_t buffer_ref = flatcc_builder_end_buffer(B, request_ref);
    if (!buffer_ref) {
        flatcc_builder_clear(B);
        printf("Failed to end buffer for %s\n", test_name);
        return 0;
    }
    
    // Get buffer and validate
    size_t buffer_size = flatcc_builder_get_buffer_size(B);
    void *buffer = flatcc_builder_get_direct_buffer(B, &buffer_size);
    
    if (!buffer) {
        flatcc_builder_clear(B);
        printf("Failed to get direct buffer for %s\n", test_name);
        return 0;
    }
    
    if (buffer_size == 0) {
        flatcc_builder_clear(B);
        printf("Zero buffer size for %s\n", test_name);
        return 0;
    }
    
    // Try to parse it back
    model_GenerateRequest_table_t request = model_GenerateRequest_as_root(buffer);
    if (!request) {
        flatcc_builder_clear(B);
        printf("Failed to parse back request for %s\n", test_name);
        return 0;
    }
    
    model_Options_table_t options = model_GenerateRequest_options(request);
    if (!options) {
        flatcc_builder_clear(B);
        printf("Failed to get options for %s\n", test_name);
        return 0;
    }
    
    flatbuffers_string_t parsed_name = model_Options_name(options);
    
    // Validate the string content
    if (test_string == NULL) {
        // NULL string should result in no name field
        if (parsed_name != NULL) {
            flatcc_builder_clear(B);
            printf("Expected NULL string but got '%s' for %s\n", parsed_name, test_name);
            return 0;
        }
    } else {
        // Non-NULL string should match
        if (!parsed_name) {
            flatcc_builder_clear(B);
            printf("Expected string but got NULL for %s\n", test_name);
            return 0;
        }
        
        size_t parsed_len = flatbuffers_string_len(parsed_name);
        size_t expected_len = strlen(test_string);
        
        if (parsed_len != expected_len) {
            flatcc_builder_clear(B);
            printf("Length mismatch for %s: expected %zu, got %zu\n", test_name, expected_len, parsed_len);
            return 0;
        }
        
        if (strcmp(parsed_name, test_string) != 0) {
            flatcc_builder_clear(B);
            printf("Content mismatch for %s: expected '%s', got '%s'\n", test_name, test_string, parsed_name);
            return 0;
        }
    }
    
    flatcc_builder_clear(B);
    printf("✅ %s: String handling test passed\n", test_name);
    return 1;
}

// Task #4 Tests - FlatBuffer String Handling

TEST_CASE(flatbuffer_string_null_handling) {
    test_setup();
    
    // Test NULL string handling
    int result = test_string_creation(NULL, "NULL string");
    TEST_ASSERT_TRUE_MESSAGE(result, "NULL string should be handled correctly");
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "NULL string test leaked memory");
    
    return test_teardown();
}

TEST_CASE(flatbuffer_string_empty_handling) {
    test_setup();
    
    // Test empty string handling
    int result = test_string_creation("", "Empty string");
    TEST_ASSERT_TRUE_MESSAGE(result, "Empty string should be handled correctly");
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "Empty string test leaked memory");
    
    return test_teardown();
}

TEST_CASE(flatbuffer_string_normal_handling) {
    test_setup();
    
    // Test normal string handling
    int result = test_string_creation("Normal test string", "Normal string");
    TEST_ASSERT_TRUE_MESSAGE(result, "Normal string should be handled correctly");
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "Normal string test leaked memory");
    
    return test_teardown();
}

TEST_CASE(flatbuffer_string_long_handling) {
    test_setup();
    
    // Test long string handling (but not too long to avoid the flatcc limit we found)
    char long_string[500]; // Safe size based on Task #3 findings
    memset(long_string, 'A', sizeof(long_string) - 1);
    long_string[sizeof(long_string) - 1] = '\0';
    
    int result = test_string_creation(long_string, "Long string (500 chars)");
    TEST_ASSERT_TRUE_MESSAGE(result, "Long string should be handled correctly");
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "Long string test leaked memory");
    
    return test_teardown();
}

TEST_CASE(flatbuffer_string_special_chars) {
    test_setup();
    
    // Test special characters and UTF-8
    const char *special_string = "Special chars: áéíóú ñ €£¥ 中文 🚀";
    
    int result = test_string_creation(special_string, "Special characters");
    TEST_ASSERT_TRUE_MESSAGE(result, "Special characters should be handled correctly");
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "Special characters test leaked memory");
    
    return test_teardown();
}

TEST_CASE(flatbuffer_string_boundary_sizes) {
    test_setup();
    
    // Test various boundary sizes to understand limits better
    size_t test_sizes[] = {1, 10, 50, 100, 200, 400, 800}; // Progressive sizes
    size_t num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    for (size_t i = 0; i < num_sizes; i++) {
        char *test_string = malloc(test_sizes[i] + 1);
        TEST_ASSERT_NOT_NULL(test_string);
        
        memset(test_string, 'B', test_sizes[i]);
        test_string[test_sizes[i]] = '\0';
        
        char test_name[100];
        snprintf(test_name, sizeof(test_name), "Boundary test %zu chars", test_sizes[i]);
        
        int result = test_string_creation(test_string, test_name);
        TEST_ASSERT_TRUE_MESSAGE(result, test_name);
        
        free(test_string);
    }
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "Boundary size tests leaked memory");
    
    return test_teardown();
}

TEST_CASE(flatbuffer_multiple_strings) {
    test_setup();
    
    // Test multiple strings in the same buffer
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    memset(B, 0, sizeof(flatcc_builder_t));
    TEST_ASSERT_EQUAL_MESSAGE(0, flatcc_builder_init(B), "Failed to initialize builder");
    TEST_ASSERT_EQUAL_MESSAGE(0, flatcc_builder_start_buffer(B, 0, 0, 0), "Failed to start buffer");
    
    // Create multiple strings
    flatbuffers_string_ref_t name_ref = flatbuffers_string_create_str(B, "Test Name");
    flatbuffers_string_ref_t email_ref = flatbuffers_string_create_str(B, "test@example.com");
    flatbuffers_string_ref_t comment_ref = flatbuffers_string_create_str(B, "Test Comment");
    
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, name_ref, "Name string creation failed");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, email_ref, "Email string creation failed");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, comment_ref, "Comment string creation failed");
    
    // Create Options with all strings
    model_Options_start(B);
    model_Options_name_add(B, name_ref);
    model_Options_email_add(B, email_ref);
    model_Options_comment_add(B, comment_ref);
    model_Options_ref_t options_ref = model_Options_end(B);
    
    // Create GenerateRequest
    model_GenerateRequest_ref_t request_ref = model_GenerateRequest_create(B, options_ref);
    
    // End buffer
    flatbuffers_buffer_ref_t buffer_ref = flatcc_builder_end_buffer(B, request_ref);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, buffer_ref, "Failed to end buffer");
    
    // Get and validate buffer
    size_t buffer_size = flatcc_builder_get_buffer_size(B);
    void *buffer = flatcc_builder_get_direct_buffer(B, &buffer_size);
    
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE_MESSAGE(buffer_size > 0, "Buffer size should be positive");
    
    // Parse back and validate
    model_GenerateRequest_table_t request = model_GenerateRequest_as_root(buffer);
    TEST_ASSERT_NOT_NULL(request);
    
    model_Options_table_t options = model_GenerateRequest_options(request);
    TEST_ASSERT_NOT_NULL(options);
    
    flatbuffers_string_t parsed_name = model_Options_name(options);
    flatbuffers_string_t parsed_email = model_Options_email(options);
    flatbuffers_string_t parsed_comment = model_Options_comment(options);
    
    TEST_ASSERT_NOT_NULL(parsed_name);
    TEST_ASSERT_NOT_NULL(parsed_email);
    TEST_ASSERT_NOT_NULL(parsed_comment);
    
    TEST_ASSERT_STRING_EQUAL("Test Name", parsed_name);
    TEST_ASSERT_STRING_EQUAL("test@example.com", parsed_email);
    TEST_ASSERT_STRING_EQUAL("Test Comment", parsed_comment);
    
    flatcc_builder_clear(B);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "Multiple strings test leaked memory");
    
    return test_teardown();
}

// Main test runner
int main(void) {
    printf("=== FlatBuffer String Handling Test Suite ===\n");
    printf("Testing Task #4: FlatBuffer string serialization edge cases\n\n");
    
    RUN_TEST(flatbuffer_string_null_handling);
    RUN_TEST(flatbuffer_string_empty_handling);
    RUN_TEST(flatbuffer_string_normal_handling);
    RUN_TEST(flatbuffer_string_long_handling);
    RUN_TEST(flatbuffer_string_special_chars);
    RUN_TEST(flatbuffer_string_boundary_sizes);
    RUN_TEST(flatbuffer_multiple_strings);
    
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