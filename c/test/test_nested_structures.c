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

// Helper function to create a complete nested structure for testing
static int create_and_validate_nested_structure(
    const char *name, const char *email, const char *comment, const char *passphrase,
    int algorithm, int curve, int hash, int cipher, int compression, 
    int compression_level, int rsa_bits, const char *test_name) {
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    // Initialize builder
    memset(B, 0, sizeof(flatcc_builder_t));
    if (flatcc_builder_init(B)) {
        printf("Failed to initialize builder for %s\n", test_name);
        return 0;
    }
    
    if (flatcc_builder_start_buffer(B, 0, 0, 0)) {
        flatcc_builder_clear(B);
        printf("Failed to start buffer for %s\n", test_name);
        return 0;
    }
    
    // Create string references (only if not NULL)
    flatbuffers_string_ref_t name_ref = 0;
    flatbuffers_string_ref_t email_ref = 0;
    flatbuffers_string_ref_t comment_ref = 0;
    flatbuffers_string_ref_t passphrase_ref = 0;
    
    if (name) {
        name_ref = flatbuffers_string_create_str(B, name);
        if (name_ref == 0) {
            flatcc_builder_clear(B);
            printf("Failed to create name string for %s\n", test_name);
            return 0;
        }
    }
    
    if (email) {
        email_ref = flatbuffers_string_create_str(B, email);
        if (email_ref == 0) {
            flatcc_builder_clear(B);
            printf("Failed to create email string for %s\n", test_name);
            return 0;
        }
    }
    
    if (comment) {
        comment_ref = flatbuffers_string_create_str(B, comment);
        if (comment_ref == 0) {
            flatcc_builder_clear(B);
            printf("Failed to create comment string for %s\n", test_name);
            return 0;
        }
    }
    
    if (passphrase) {
        passphrase_ref = flatbuffers_string_create_str(B, passphrase);
        if (passphrase_ref == 0) {
            flatcc_builder_clear(B);
            printf("Failed to create passphrase string for %s\n", test_name);
            return 0;
        }
    }
    
    // Create nested KeyOptions structure
    model_KeyOptions_start(B);
    model_KeyOptions_algorithm_add(B, (model_Algorithm_enum_t)algorithm);
    model_KeyOptions_curve_add(B, (model_Curve_enum_t)curve);
    model_KeyOptions_hash_add(B, (model_Hash_enum_t)hash);
    model_KeyOptions_cipher_add(B, (model_Cipher_enum_t)cipher);
    model_KeyOptions_compression_add(B, (model_Compression_enum_t)compression);
    model_KeyOptions_compression_level_add(B, compression_level);
    model_KeyOptions_rsa_bits_add(B, rsa_bits);
    model_KeyOptions_ref_t key_options_ref = model_KeyOptions_end(B);
    
    if (key_options_ref == 0) {
        flatcc_builder_clear(B);
        printf("Failed to create KeyOptions for %s\n", test_name);
        return 0;
    }
    
    // Create Options structure with nested KeyOptions
    model_Options_start(B);
    if (name_ref) model_Options_name_add(B, name_ref);
    if (email_ref) model_Options_email_add(B, email_ref);
    if (comment_ref) model_Options_comment_add(B, comment_ref);
    if (passphrase_ref) model_Options_passphrase_add(B, passphrase_ref);
    model_Options_key_options_add(B, key_options_ref);
    model_Options_ref_t options_ref = model_Options_end(B);
    
    if (options_ref == 0) {
        flatcc_builder_clear(B);
        printf("Failed to create Options for %s\n", test_name);
        return 0;
    }
    
    // Create top-level GenerateRequest
    model_GenerateRequest_ref_t request_ref = model_GenerateRequest_create(B, options_ref);
    if (request_ref == 0) {
        flatcc_builder_clear(B);
        printf("Failed to create GenerateRequest for %s\n", test_name);
        return 0;
    }
    
    // End buffer
    flatbuffers_buffer_ref_t buffer_ref = flatcc_builder_end_buffer(B, request_ref);
    if (!buffer_ref) {
        flatcc_builder_clear(B);
        printf("Failed to end buffer for %s\n", test_name);
        return 0;
    }
    
    // Get buffer
    size_t buffer_size = flatcc_builder_get_buffer_size(B);
    void *buffer = flatcc_builder_get_direct_buffer(B, &buffer_size);
    
    if (!buffer || buffer_size == 0) {
        flatcc_builder_clear(B);
        printf("Failed to get buffer for %s (size: %zu)\n", test_name, buffer_size);
        return 0;
    }
    
    // Validate by parsing back
    model_GenerateRequest_table_t parsed_request = model_GenerateRequest_as_root(buffer);
    if (!parsed_request) {
        flatcc_builder_clear(B);
        printf("Failed to parse request for %s\n", test_name);
        return 0;
    }
    
    model_Options_table_t parsed_options = model_GenerateRequest_options(parsed_request);
    if (!parsed_options) {
        flatcc_builder_clear(B);
        printf("Failed to get parsed options for %s\n", test_name);
        return 0;
    }
    
    model_KeyOptions_table_t parsed_key_options = model_Options_key_options(parsed_options);
    if (!parsed_key_options) {
        flatcc_builder_clear(B);
        printf("Failed to get parsed key options for %s\n", test_name);
        return 0;
    }
    
    // Validate string fields
    if (name) {
        flatbuffers_string_t parsed_name = model_Options_name(parsed_options);
        if (!parsed_name || strcmp(parsed_name, name) != 0) {
            flatcc_builder_clear(B);
            printf("Name validation failed for %s\n", test_name);
            return 0;
        }
    }
    
    if (email) {
        flatbuffers_string_t parsed_email = model_Options_email(parsed_options);
        if (!parsed_email || strcmp(parsed_email, email) != 0) {
            flatcc_builder_clear(B);
            printf("Email validation failed for %s\n", test_name);
            return 0;
        }
    }
    
    // Validate KeyOptions fields
    if (model_KeyOptions_algorithm(parsed_key_options) != algorithm ||
        model_KeyOptions_curve(parsed_key_options) != curve ||
        model_KeyOptions_hash(parsed_key_options) != hash ||
        model_KeyOptions_cipher(parsed_key_options) != cipher ||
        model_KeyOptions_compression(parsed_key_options) != compression ||
        model_KeyOptions_compression_level(parsed_key_options) != compression_level ||
        model_KeyOptions_rsa_bits(parsed_key_options) != rsa_bits) {
        flatcc_builder_clear(B);
        printf("KeyOptions validation failed for %s\n", test_name);
        return 0;
    }
    
    flatcc_builder_clear(B);
    printf("✅ %s: Nested structure test passed (buffer size: %zu)\n", test_name, buffer_size);
    return 1;
}

// Task #5 Tests - Nested FlatBuffer Structures

TEST_CASE(nested_structure_minimal) {
    test_setup();
    
    // Test minimal nested structure
    int result = create_and_validate_nested_structure(
        "Test", "test@example.com", NULL, NULL,
        1, 0, 1, 1, 1, 6, 2048, "Minimal nested structure"
    );
    
    TEST_ASSERT_TRUE_MESSAGE(result, "Minimal nested structure should work");
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Minimal nested test leaked memory");
    
    return test_teardown();
}

TEST_CASE(nested_structure_complete) {
    test_setup();
    
    // Test complete nested structure with all fields
    int result = create_and_validate_nested_structure(
        "John Doe", "john.doe@example.com", "Test Comment", "secret123",
        1, 2, 3, 2, 1, 9, 4096, "Complete nested structure"
    );
    
    TEST_ASSERT_TRUE_MESSAGE(result, "Complete nested structure should work");
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Complete nested test leaked memory");
    
    return test_teardown();
}

TEST_CASE(nested_structure_optional_fields) {
    test_setup();
    
    // Test with some optional fields as NULL
    int result = create_and_validate_nested_structure(
        "Alice", "alice@test.com", NULL, "password",
        2, 1, 2, 3, 2, 5, 3072, "Optional fields nested structure"
    );
    
    TEST_ASSERT_TRUE_MESSAGE(result, "Nested structure with optional NULL fields should work");
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Optional fields test leaked memory");
    
    return test_teardown();
}

TEST_CASE(nested_structure_multiple_create) {
    test_setup();
    
    // Test creating multiple nested structures in sequence
    for (int i = 0; i < 5; i++) {
        char name[50], email[50], comment[50];
        snprintf(name, sizeof(name), "User%d", i);
        snprintf(email, sizeof(email), "user%d@test.com", i);
        snprintf(comment, sizeof(comment), "Comment for user %d", i);
        
        int result = create_and_validate_nested_structure(
            name, email, comment, "pass123",
            (i % 3) + 1, i % 4, (i % 2) + 1, (i % 3) + 1, 
            (i % 3) + 1, (i % 10) + 1, 2048 + (i * 512), 
            "Multiple nested structures"
        );
        
        TEST_ASSERT_TRUE_MESSAGE(result, "Multiple nested structures should all work");
    }
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Multiple nested test leaked memory");
    
    return test_teardown();
}

TEST_CASE(nested_structure_edge_values) {
    test_setup();
    
    // Test with edge case values
    int result = create_and_validate_nested_structure(
        "", "", "", "",  // Empty strings
        0, 0, 0, 0, 0, 0, 1024,  // Minimum values
        "Edge values nested structure"
    );
    
    TEST_ASSERT_TRUE_MESSAGE(result, "Nested structure with edge values should work");
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Edge values test leaked memory");
    
    return test_teardown();
}

TEST_CASE(nested_structure_large_strings) {
    test_setup();
    
    // Test with moderately large strings (safe based on Task #3 findings)
    char large_name[300];
    char large_email[300];
    char large_comment[400];
    
    memset(large_name, 'N', sizeof(large_name) - 1);
    large_name[sizeof(large_name) - 1] = '\0';
    
    memset(large_email, 'E', sizeof(large_email) - 1);
    large_email[sizeof(large_email) - 1] = '\0';
    
    memset(large_comment, 'C', sizeof(large_comment) - 1);
    large_comment[sizeof(large_comment) - 1] = '\0';
    
    int result = create_and_validate_nested_structure(
        large_name, large_email, large_comment, "secret",
        3, 2, 3, 1, 2, 8, 2048, "Large strings nested structure"
    );
    
    TEST_ASSERT_TRUE_MESSAGE(result, "Nested structure with large strings should work");
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Large strings nested test leaked memory");
    
    return test_teardown();
}

// Test for proper cleanup on failure paths
TEST_CASE(nested_structure_builder_state) {
    test_setup();
    
    // Test that multiple builder operations don't interfere
    flatcc_builder_t builder1, builder2;
    flatcc_builder_t *B1 = &builder1;
    flatcc_builder_t *B2 = &builder2;
    
    memset(B1, 0, sizeof(flatcc_builder_t));
    memset(B2, 0, sizeof(flatcc_builder_t));
    
    TEST_ASSERT_EQUAL_MESSAGE(0, flatcc_builder_init(B1), "First builder init failed");
    TEST_ASSERT_EQUAL_MESSAGE(0, flatcc_builder_init(B2), "Second builder init failed");
    
    TEST_ASSERT_EQUAL_MESSAGE(0, flatcc_builder_start_buffer(B1, 0, 0, 0), "First buffer start failed");
    TEST_ASSERT_EQUAL_MESSAGE(0, flatcc_builder_start_buffer(B2, 0, 0, 0), "Second buffer start failed");
    
    // Create simple structures in both builders
    flatbuffers_string_ref_t str1 = flatbuffers_string_create_str(B1, "Builder1");
    flatbuffers_string_ref_t str2 = flatbuffers_string_create_str(B2, "Builder2");
    
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, str1, "First string creation failed");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, str2, "Second string creation failed");
    
    // Create nested structures in both
    model_KeyOptions_start(B1);
    model_KeyOptions_algorithm_add(B1, 1);
    model_KeyOptions_ref_t key_opts1 = model_KeyOptions_end(B1);
    
    model_KeyOptions_start(B2);
    model_KeyOptions_algorithm_add(B2, 2);
    model_KeyOptions_ref_t key_opts2 = model_KeyOptions_end(B2);
    
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, key_opts1, "First KeyOptions creation failed");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, key_opts2, "Second KeyOptions creation failed");
    
    flatcc_builder_clear(B1);
    flatcc_builder_clear(B2);
    
    TEST_ASSERT_FALSE_MESSAGE(memory_tracking_has_leaks(), "Builder state test leaked memory");
    
    return test_teardown();
}

// Main test runner
int main(void) {
    printf("=== Nested FlatBuffer Structures Test Suite ===\n");
    printf("Testing Task #5: Complex nested structures (Options + KeyOptions)\n\n");
    
    RUN_TEST(nested_structure_minimal);
    RUN_TEST(nested_structure_complete);
    RUN_TEST(nested_structure_optional_fields);
    RUN_TEST(nested_structure_multiple_create);
    RUN_TEST(nested_structure_edge_values);
    RUN_TEST(nested_structure_large_strings);
    RUN_TEST(nested_structure_builder_state);
    
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