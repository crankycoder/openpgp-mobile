/**
 * Memory Regression Test Suite - Large Data Testing
 * Tests size limit boundary conditions to prevent memory issues
 * Based on empirical limits from Phase 7.6 FlatCC analysis
 */

#include "test_framework_isolated.h"
#include "test_isolation_manager.h"
#include "../include/openpgp.h"
#include <stdlib.h>
#include <string.h>

// Test data size boundaries around critical limits
static int test_message_size_boundaries(void) {
    printf("Testing message size boundaries...\n");
    
    // Test just under 2KB limit (should succeed with bridge error)
    char *small_msg = malloc(2047);
    memset(small_msg, 'A', 2046);
    small_msg[2046] = '\0';
    
    openpgp_result_t result = openpgp_encrypt_symmetric(small_msg, "password123", NULL, NULL);
    TEST_ASSERT(result.error == OPENPGP_ERROR_BRIDGE_CALL); // Expected bridge error, not size error
    
    free(small_msg);
    
    // Test exactly at 2KB limit (should succeed with bridge error)
    char *exact_msg = malloc(2049);
    memset(exact_msg, 'B', 2048);
    exact_msg[2048] = '\0';
    
    result = openpgp_encrypt_symmetric(exact_msg, "password123", NULL, NULL);
    TEST_ASSERT(result.error == OPENPGP_ERROR_BRIDGE_CALL); // Expected bridge error, not size error
    
    free(exact_msg);
    
    // Test just over 2KB limit (should fail with size error)
    char *large_msg = malloc(2050);
    memset(large_msg, 'C', 2049);
    large_msg[2049] = '\0';
    
    result = openpgp_encrypt_symmetric(large_msg, "password123", NULL, NULL);
    TEST_ASSERT(result.error == OPENPGP_ERROR_SIZE_LIMIT); // Should fail with size limit error
    
    free(large_msg);
    
    printf("Message size boundary tests passed\n");
    return 0;
}static int test_signature_data_boundaries(void) {
    printf("Testing signature data size boundaries...\n");
    
    // Test just under 3KB limit
    char *small_data = malloc(3071);
    memset(small_data, 'X', 3070);
    small_data[3070] = '\0';
    
    openpgp_result_t result = openpgp_sign(small_data, "test_key", NULL, NULL);
    TEST_ASSERT(result.error == OPENPGP_ERROR_BRIDGE_CALL); // Expected bridge error, not size error
    
    free(small_data);
    
    // Test exactly at 3KB limit
    char *exact_data = malloc(3073);
    memset(exact_data, 'Y', 3072);
    exact_data[3072] = '\0';
    
    result = openpgp_sign(exact_data, "test_key", NULL, NULL);
    TEST_ASSERT(result.error == OPENPGP_ERROR_BRIDGE_CALL); // Expected bridge error, not size error
    
    free(exact_data);
    
    // Test just over 3KB limit (should fail with size error)
    char *large_data = malloc(3074);
    memset(large_data, 'Z', 3073);
    large_data[3073] = '\0';
    
    result = openpgp_sign(large_data, "test_key", NULL, NULL);
    TEST_ASSERT(result.error == OPENPGP_ERROR_SIZE_LIMIT); // Should fail with size limit error
    
    free(large_data);
    
    printf("Signature data size boundary tests passed\n");
    return 0;
}

static int test_key_parameter_boundaries(void) {
    printf("Testing key parameter size boundaries...\n");
    
    // Test just under 512B limit
    char *small_comment = malloc(511);
    memset(small_comment, 'K', 510);
    small_comment[510] = '\0';
    
    openpgp_result_t result = openpgp_generate_key(small_comment, "test@example.com", "password");
    TEST_ASSERT(result.error == OPENPGP_ERROR_BRIDGE_CALL); // Expected bridge error, not size error
    
    free(small_comment);
    
    // Test exactly at 512B limit
    char *exact_comment = malloc(513);
    memset(exact_comment, 'L', 512);
    exact_comment[512] = '\0';
    
    result = openpgp_generate_key(exact_comment, "test@example.com", "password");
    TEST_ASSERT(result.error == OPENPGP_ERROR_BRIDGE_CALL); // Expected bridge error, not size error
    
    free(exact_comment);    // Test just over 512B limit (should fail with size error)
    char *large_comment = malloc(514);
    memset(large_comment, 'M', 513);
    large_comment[513] = '\0';
    
    result = openpgp_generate_key(large_comment, "test@example.com", "password");
    TEST_ASSERT(result.error == OPENPGP_ERROR_SIZE_LIMIT); // Should fail with size limit error
    
    free(large_comment);
    
    printf("Key parameter size boundary tests passed\n");
    return 0;
}

static int test_flatbuffer_size_boundaries(void) {
    printf("Testing FlatBuffer size boundaries (4KB limit)...\n");
    
    // Test large message that would create oversized FlatBuffer
    char *huge_msg = malloc(4097);
    memset(huge_msg, 'F', 4096);
    huge_msg[4096] = '\0';
    
    openpgp_result_t result = openpgp_encrypt_symmetric(huge_msg, "password123", NULL, NULL);
    TEST_ASSERT(result.error == OPENPGP_ERROR_SIZE_LIMIT); // Should fail due to FlatBuffer size
    
    free(huge_msg);
    
    // Test signature data that would exceed FlatBuffer limits
    char *huge_data = malloc(4097);
    memset(huge_data, 'G', 4096);
    huge_data[4096] = '\0';
    
    result = openpgp_sign(huge_data, "test_key", NULL, NULL);
    TEST_ASSERT(result.error == OPENPGP_ERROR_SIZE_LIMIT); // Should fail due to FlatBuffer size
    
    free(huge_data);
    
    printf("FlatBuffer size boundary tests passed\n");
    return 0;
}

static int test_memory_allocation_patterns(void) {
    printf("Testing memory allocation patterns under size limits...\n");
    
    // Test multiple allocations within limits
    for (int i = 0; i < 10; i++) {
        char *msg = malloc(1024); // 1KB each, well within limits
        memset(msg, 'T', 1023);
        msg[1023] = '\0';
        
        openpgp_result_t result = openpgp_encrypt_symmetric(msg, "test", NULL, NULL);
        TEST_ASSERT(result.error == OPENPGP_ERROR_BRIDGE_CALL); // Expected bridge error
        
        free(msg);
    }    // Test gradual size increase to find exact breaking point
    for (size_t size = 2000; size <= 2100; size += 10) {
        char *msg = malloc(size + 1);
        memset(msg, 'S', size);
        msg[size] = '\0';
        
        openpgp_result_t result = openpgp_encrypt_symmetric(msg, "test", NULL, NULL);
        
        if (size <= 2048) {
            TEST_ASSERT(result.error == OPENPGP_ERROR_BRIDGE_CALL); // Should succeed size validation
        } else {
            TEST_ASSERT(result.error == OPENPGP_ERROR_SIZE_LIMIT); // Should fail size validation
        }
        
        free(msg);
    }
    
    printf("Memory allocation pattern tests passed\n");
    return 0;
}

static int test_concurrent_size_validation(void) {
    printf("Testing concurrent size validation scenarios...\n");
    
    // Simulate concurrent operations with different sizes
    char *msg1 = malloc(1024);
    char *msg2 = malloc(2049); // Just over limit
    char *msg3 = malloc(512);
    
    memset(msg1, 'P', 1023);
    msg1[1023] = '\0';
    memset(msg2, 'Q', 2048);
    msg2[2048] = '\0';
    memset(msg3, 'R', 511);
    msg3[511] = '\0';
    
    // These should behave independently
    openpgp_result_t result1 = openpgp_encrypt_symmetric(msg1, "pass1", NULL, NULL);
    openpgp_result_t result2 = openpgp_encrypt_symmetric(msg2, "pass2", NULL, NULL);
    openpgp_result_t result3 = openpgp_encrypt_symmetric(msg3, "pass3", NULL, NULL);
    
    TEST_ASSERT(result1.error == OPENPGP_ERROR_BRIDGE_CALL); // Within limits
    TEST_ASSERT(result2.error == OPENPGP_ERROR_SIZE_LIMIT);  // Over limit
    TEST_ASSERT(result3.error == OPENPGP_ERROR_BRIDGE_CALL); // Within limits
    
    free(msg1);
    free(msg2);
    free(msg3);
    
    printf("Concurrent size validation tests passed\n");
    return 0;
}int main() {
    printf("\n=== Memory Regression Test Suite - Large Data Testing ===\n");
    printf("Testing size limit boundary conditions for memory safety\n\n");
    
    int total_failed = 0;
    
    total_failed += run_isolated_test(test_message_size_boundaries, "Message Size Boundaries");
    total_failed += run_isolated_test(test_signature_data_boundaries, "Signature Data Boundaries");
    total_failed += run_isolated_test(test_key_parameter_boundaries, "Key Parameter Boundaries");
    total_failed += run_isolated_test(test_flatbuffer_size_boundaries, "FlatBuffer Size Boundaries");
    total_failed += run_isolated_test(test_memory_allocation_patterns, "Memory Allocation Patterns");
    total_failed += run_isolated_test(test_concurrent_size_validation, "Concurrent Size Validation");
    
    printf("\n=== Large Data Testing Summary ===\n");
    printf("Tests run: %d, Tests failed: %d\n", get_tests_run(), get_tests_failed());
    
    if (total_failed > 0) {
        printf(COLOR_RED "FAILED: %d test(s) failed\n" COLOR_RESET, total_failed);
        return 1;
    }
    
    printf(COLOR_GREEN "SUCCESS: All large data boundary tests passed\n" COLOR_RESET);
    return 0;
}