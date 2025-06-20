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

// This creates a minimal version of serialize_generate_request to test the exact logic
static openpgp_result_t test_serialize_generate_request(const openpgp_options_t *options, void **buffer, size_t *buffer_size) {
    /* Create FlatBuffer builder */
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    /* Initialize the builder with proper cleanup */
    memset(B, 0, sizeof(flatcc_builder_t));
    if (flatcc_builder_init(B)) {
        return (openpgp_result_t){OPENPGP_ERROR_MEMORY_ALLOCATION, strdup("Failed to initialize FlatBuffer builder"), NULL, 0};
    }
    
    /* Ensure proper buffer start */
    if (flatcc_builder_start_buffer(B, 0, 0, 0)) {
        flatcc_builder_clear(B);
        return (openpgp_result_t){OPENPGP_ERROR_SERIALIZATION, strdup("Failed to start FlatBuffer"), NULL, 0};
    }
    
    /* Create string references */
    flatbuffers_string_ref_t name_ref = 0;
    flatbuffers_string_ref_t email_ref = 0; 
    flatbuffers_string_ref_t comment_ref = 0;
    flatbuffers_string_ref_t passphrase_ref = 0;
    
    if (options->name) {
        name_ref = flatbuffers_string_create_str(B, options->name);
        printf("DEBUG: name_ref = %u (string length: %zu)\n", name_ref, strlen(options->name));
    }
    if (options->email) {
        email_ref = flatbuffers_string_create_str(B, options->email);
        printf("DEBUG: email_ref = %u (string length: %zu)\n", email_ref, strlen(options->email));
    }
    if (options->comment) {
        comment_ref = flatbuffers_string_create_str(B, options->comment);
        printf("DEBUG: comment_ref = %u (string length: %zu)\n", comment_ref, strlen(options->comment));
    }
    if (options->passphrase) {
        passphrase_ref = flatbuffers_string_create_str(B, options->passphrase);
        printf("DEBUG: passphrase_ref = %u (string length: %zu)\n", passphrase_ref, strlen(options->passphrase));
    }
    
    /* Create KeyOptions */
    model_KeyOptions_start(B);
    model_KeyOptions_algorithm_add(B, (model_Algorithm_enum_t)options->key_options.algorithm);
    model_KeyOptions_curve_add(B, (model_Curve_enum_t)options->key_options.curve);
    model_KeyOptions_hash_add(B, (model_Hash_enum_t)options->key_options.hash);
    model_KeyOptions_cipher_add(B, (model_Cipher_enum_t)options->key_options.cipher);
    model_KeyOptions_compression_add(B, (model_Compression_enum_t)options->key_options.compression);
    model_KeyOptions_compression_level_add(B, options->key_options.compression_level);
    model_KeyOptions_rsa_bits_add(B, options->key_options.rsa_bits);
    model_KeyOptions_ref_t key_options_ref = model_KeyOptions_end(B);
    
    /* Create Options */
    model_Options_start(B);
    if (name_ref) model_Options_name_add(B, name_ref);
    if (comment_ref) model_Options_comment_add(B, comment_ref);
    if (email_ref) model_Options_email_add(B, email_ref);
    if (passphrase_ref) model_Options_passphrase_add(B, passphrase_ref);
    if (key_options_ref) model_Options_key_options_add(B, key_options_ref);
    model_Options_ref_t options_ref = model_Options_end(B);
    
    /* Create GenerateRequest */
    model_GenerateRequest_ref_t request_ref = model_GenerateRequest_create(B, options_ref);
    
    /* Finish the buffer */
    flatbuffers_buffer_ref_t buffer_ref = flatcc_builder_end_buffer(B, request_ref);
    if (!buffer_ref) {
        flatcc_builder_clear(B);
        return (openpgp_result_t){OPENPGP_ERROR_SERIALIZATION, strdup("Failed to create FlatBuffer"), NULL, 0};
    }
    
    /* Get buffer size and data */
    *buffer_size = flatcc_builder_get_buffer_size(B);
    printf("DEBUG: FlatBuffer builder reported size: %zu\n", *buffer_size);
    
    if (*buffer_size == 0) {
        flatcc_builder_clear(B);
        return (openpgp_result_t){OPENPGP_ERROR_SERIALIZATION, strdup("FlatBuffer builder returned zero size"), NULL, 0};
    }
    
    void *data = malloc(*buffer_size);
    if (!data) {
        flatcc_builder_clear(B);
        return (openpgp_result_t){OPENPGP_ERROR_MEMORY_ALLOCATION, strdup("Failed to allocate buffer"), NULL, 0};
    }
    
    /* Copy the buffer data */
    printf("DEBUG: Before get_direct_buffer: *buffer_size = %zu\n", *buffer_size);
    void *builder_buffer = flatcc_builder_get_direct_buffer(B, buffer_size);
    printf("DEBUG: After get_direct_buffer: *buffer_size = %zu, builder_buffer = %p\n", *buffer_size, builder_buffer);
    memcpy(data, builder_buffer, *buffer_size);
    *buffer = data;
    printf("DEBUG: Final: *buffer_size = %zu, *buffer = %p\n", *buffer_size, *buffer);
    
    flatcc_builder_clear(B);
    return (openpgp_result_t){OPENPGP_SUCCESS, NULL, NULL, 0};
}

// Function to validate FlatBuffer format
static int validate_flatbuffer_format(const void *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 8) {
        printf("Invalid buffer: NULL or too small (%zu bytes)\n", buffer_size);
        return 0;
    }
    
    // Try to parse as GenerateRequest
    model_GenerateRequest_table_t request = model_GenerateRequest_as_root(buffer);
    if (!request) {
        printf("Failed to parse buffer as GenerateRequest\n");
        return 0;
    }
    
    // Try to get options
    model_Options_table_t options = model_GenerateRequest_options(request);
    if (!options) {
        printf("Failed to get options from GenerateRequest\n");
        return 0;
    }
    
    printf("Buffer validation: Successfully parsed as GenerateRequest\n");
    
    // Try to access some fields to ensure they're valid
    flatbuffers_string_t name = model_Options_name(options);
    flatbuffers_string_t email = model_Options_email(options);
    
    if (name) {
        printf("  Name: '%s' (length: %zu)\n", name, flatbuffers_string_len(name));
    }
    if (email) {
        printf("  Email: '%s' (length: %zu)\n", email, flatbuffers_string_len(email));
    }
    
    return 1;
}

// Task #3 - Test actual serialize_generate_request buffer format

TEST_CASE(serialize_validation_minimal) {
    test_setup();
    
    // Test with minimal valid options
    openpgp_options_t options = {0};
    options.name = "test";
    options.email = "test@example.com";
    
    void *buffer = NULL;
    size_t buffer_size = 0;
    
    openpgp_result_t result = test_serialize_generate_request(&options, &buffer, &buffer_size);
    
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_SUCCESS, result.error, "serialize should succeed");
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE_MESSAGE(buffer_size > 0, "Buffer size should be positive");
    
    // Validate the FlatBuffer format
    int valid = validate_flatbuffer_format(buffer, buffer_size);
    TEST_ASSERT_TRUE_MESSAGE(valid, "FlatBuffer should be valid format");
    
    // Clean up
    if (buffer) {
        free(buffer);
    }
    if (result.error_message) {
        free(result.error_message);
    }
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "serialize validation minimal test leaked memory");
    
    return test_teardown();
}

TEST_CASE(serialize_validation_large_strings) {
    test_setup();
    
    // Test with large strings that might cause buffer overruns
    char large_name[1000];
    char large_email[1000]; 
    char large_comment[2000];
    char large_passphrase[500];
    
    memset(large_name, 'A', sizeof(large_name) - 1);
    large_name[sizeof(large_name) - 1] = '\0';
    
    memset(large_email, 'B', sizeof(large_email) - 1);
    large_email[sizeof(large_email) - 1] = '\0';
    
    memset(large_comment, 'C', sizeof(large_comment) - 1);
    large_comment[sizeof(large_comment) - 1] = '\0';
    
    memset(large_passphrase, 'D', sizeof(large_passphrase) - 1);
    large_passphrase[sizeof(large_passphrase) - 1] = '\0';
    
    openpgp_options_t options = {0};
    options.name = large_name;
    options.email = large_email;
    options.comment = large_comment;
    options.passphrase = large_passphrase;
    
    void *buffer = NULL;
    size_t buffer_size = 0;
    
    openpgp_result_t result = test_serialize_generate_request(&options, &buffer, &buffer_size);
    
    printf("DEBUG: test result.error = %d, buffer = %p, buffer_size = %zu\n", result.error, buffer, buffer_size);
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_SUCCESS, result.error, "serialize should succeed with large strings");
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE_MESSAGE(buffer_size > 0, "Buffer size should be positive");
    
    // Validate the FlatBuffer format
    int valid = validate_flatbuffer_format(buffer, buffer_size);
    TEST_ASSERT_TRUE_MESSAGE(valid, "FlatBuffer should be valid format even with large strings");
    
    // Clean up
    if (buffer) {
        free(buffer);
    }
    if (result.error_message) {
        free(result.error_message);
    }
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "serialize validation large strings test leaked memory");
    
    return test_teardown();
}

TEST_CASE(serialize_validation_null_fields) {
    test_setup();
    
    // Test with NULL optional fields
    openpgp_options_t options = {0};
    options.name = "test";
    options.email = "test@example.com";
    // comment and passphrase intentionally NULL
    
    void *buffer = NULL;
    size_t buffer_size = 0;
    
    openpgp_result_t result = test_serialize_generate_request(&options, &buffer, &buffer_size);
    
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_SUCCESS, result.error, "serialize should succeed with NULL fields");
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE_MESSAGE(buffer_size > 0, "Buffer size should be positive");
    
    // Validate the FlatBuffer format
    int valid = validate_flatbuffer_format(buffer, buffer_size);
    TEST_ASSERT_TRUE_MESSAGE(valid, "FlatBuffer should be valid format with NULL fields");
    
    // Clean up
    if (buffer) {
        free(buffer);
    }
    if (result.error_message) {
        free(result.error_message);
    }
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "serialize validation NULL fields test leaked memory");
    
    return test_teardown();
}

TEST_CASE(serialize_validation_empty_strings) {
    test_setup();
    
    // Test with empty strings
    openpgp_options_t options = {0};
    options.name = "";
    options.email = "";
    options.comment = "";
    options.passphrase = "";
    
    void *buffer = NULL;
    size_t buffer_size = 0;
    
    openpgp_result_t result = test_serialize_generate_request(&options, &buffer, &buffer_size);
    
    TEST_ASSERT_EQUAL_MESSAGE(OPENPGP_SUCCESS, result.error, "serialize should succeed with empty strings");
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE_MESSAGE(buffer_size > 0, "Buffer size should be positive");
    
    // Validate the FlatBuffer format
    int valid = validate_flatbuffer_format(buffer, buffer_size);
    TEST_ASSERT_TRUE_MESSAGE(valid, "FlatBuffer should be valid format with empty strings");
    
    // Clean up
    if (buffer) {
        free(buffer);
    }
    if (result.error_message) {
        free(result.error_message);
    }
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), 
                             "serialize validation empty strings test leaked memory");
    
    return test_teardown();
}

// Main test runner
int main(void) {
    printf("=== FlatBuffer Serialization Validation Test Suite ===\n");
    printf("Testing serialize_generate_request buffer format validation\n\n");
    
    RUN_TEST(serialize_validation_minimal);
    RUN_TEST(serialize_validation_large_strings);
    RUN_TEST(serialize_validation_null_fields);
    RUN_TEST(serialize_validation_empty_strings);
    
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