/**
 * Memory Regression Test Suite - Stress Testing
 * Tests repeated operations to detect memory leaks and accumulation
 * Validates memory stability under sustained operations
 */

#include "test_framework_isolated.h"
#include "test_isolation_manager.h"
#include "../include/openpgp.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Helper macro to free error message and check result */
#define CHECK_RESULT_AND_FREE(result, expected_error) do { \
    if ((result).error_message) { \
        free((result).error_message); \
        (result).error_message = NULL; \
    } \
    TEST_ASSERT_EQUAL((expected_error), (result).error); \
} while(0)

#define STRESS_ITERATIONS 100
#define RAPID_ITERATIONS 1000
#define LARGE_BATCH_SIZE 50

static int test_repeated_encryption_operations(void) {
    printf("Testing repeated encryption operations (%d iterations)...\n", STRESS_ITERATIONS);
    
    const char *message = "Test message for stress testing";
    const char *password = "stress_test_password";
    
    for (int i = 0; i < STRESS_ITERATIONS; i++) {
        openpgp_result_t result = openpgp_encrypt_symmetric(message, password, NULL, NULL);
        CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_BRIDGE_CALL); // Expected bridge error
        
        // Test isolation should prevent any memory accumulation between iterations
        if (i % 10 == 0) {
            printf("Completed %d encryption operations\n", i + 1);
        }
    }
    
    printf("Repeated encryption stress test passed\n");
    return 0;
}static int test_repeated_signing_operations(void) {
    printf("Testing repeated signing operations (%d iterations)...\n", STRESS_ITERATIONS);
    
    const char *data = "Data to be signed repeatedly";
    const char *key_id = "stress_test_key";
    
    for (int i = 0; i < STRESS_ITERATIONS; i++) {
        openpgp_result_t result = openpgp_sign(data, key_id, NULL, NULL);
        CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_BRIDGE_CALL); // Expected bridge error
        
        if (i % 10 == 0) {
            printf("Completed %d signing operations\n", i + 1);
        }
    }
    
    printf("Repeated signing stress test passed\n");
    return 0;
}

static int test_repeated_key_generation(void) {
    printf("Testing repeated key generation (%d iterations)...\n", STRESS_ITERATIONS);
    
    for (int i = 0; i < STRESS_ITERATIONS; i++) {
        char name[64];
        snprintf(name, sizeof(name), "Stress test key %d", i);
        
        openpgp_result_t result = openpgp_generate_key(name, "test@example.com", "password");
        CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_BRIDGE_CALL); // Expected bridge error
        
        if (i % 10 == 0) {
            printf("Completed %d key generations\n", i + 1);
        }
    }
    
    printf("Repeated key generation stress test passed\n");
    return 0;
}

static int test_rapid_sequential_operations(void) {
    printf("Testing rapid sequential operations (%d iterations)...\n", RAPID_ITERATIONS);
    
    const char *msg = "Quick test";
    const char *pass = "quick";
    
    clock_t start = clock();
    
    for (int i = 0; i < RAPID_ITERATIONS; i++) {
        openpgp_result_t result = openpgp_encrypt_symmetric(msg, pass, NULL, NULL);
        CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_BRIDGE_CALL); // Expected bridge error
    }
    
    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;    printf("Completed %d rapid operations in %.2f seconds\n", RAPID_ITERATIONS, cpu_time);
    printf("Rapid sequential operations stress test passed\n");
    return 0;
}

static int test_mixed_operation_patterns(void) {
    printf("Testing mixed operation patterns...\n");
    
    const char *messages[] = {"Msg1", "Msg2", "Msg3", "Msg4", "Msg5"};
    const char *passwords[] = {"Pass1", "Pass2", "Pass3", "Pass4", "Pass5"};
    const char *data[] = {"Data1", "Data2", "Data3", "Data4", "Data5"};
    const char *keys[] = {"Key1", "Key2", "Key3", "Key4", "Key5"};
    
    for (int cycle = 0; cycle < 20; cycle++) {
        // Mix of different operations
        for (int i = 0; i < 5; i++) {
            openpgp_result_t result1 = openpgp_encrypt_symmetric(messages[i], passwords[i], NULL, NULL);
            CHECK_RESULT_AND_FREE(result1, OPENPGP_ERROR_BRIDGE_CALL);
            
            openpgp_result_t result2 = openpgp_sign(data[i], keys[i], NULL, NULL);
            CHECK_RESULT_AND_FREE(result2, OPENPGP_ERROR_BRIDGE_CALL);
        }
        
        if (cycle % 5 == 0) {
            printf("Completed %d mixed operation cycles\n", cycle + 1);
        }
    }
    
    printf("Mixed operation patterns stress test passed\n");
    return 0;
}

static int test_size_validation_under_stress(void) {
    printf("Testing size validation under stress conditions...\n");
    
    // Test size validation with alternating valid/invalid sizes
    for (int i = 0; i < STRESS_ITERATIONS; i++) {
        if (i % 2 == 0) {
            // Valid size
            char *valid_msg = malloc(1024);
            memset(valid_msg, 'V', 1023);
            valid_msg[1023] = '\0';
            
            openpgp_result_t result = openpgp_encrypt_symmetric(valid_msg, "test", NULL, NULL);
            CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_BRIDGE_CALL);
            
            free(valid_msg);
        } else {
            // Invalid size
            char *invalid_msg = malloc(3000);
            memset(invalid_msg, 'I', 2999);
            invalid_msg[2999] = '\0';
            
            openpgp_result_t result = openpgp_encrypt_symmetric(invalid_msg, "test", NULL, NULL);
            CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_SIZE_LIMIT);
            
            free(invalid_msg);
        }
    }    printf("Size validation under stress test passed\n");
    return 0;
}

static int test_large_batch_operations(void) {
    printf("Testing large batch operations (%d batches)...\n", LARGE_BATCH_SIZE);
    
    // Allocate arrays for batch processing
    char **messages = malloc(LARGE_BATCH_SIZE * sizeof(char*));
    char **passwords = malloc(LARGE_BATCH_SIZE * sizeof(char*));
    
    // Initialize batch data
    for (int i = 0; i < LARGE_BATCH_SIZE; i++) {
        messages[i] = malloc(512);
        passwords[i] = malloc(32);
        snprintf(messages[i], 512, "Batch message %d with some content", i);
        snprintf(passwords[i], 32, "batch_pass_%d", i);
    }
    
    // Process entire batch
    for (int i = 0; i < LARGE_BATCH_SIZE; i++) {
        openpgp_result_t result = openpgp_encrypt_symmetric(messages[i], passwords[i], NULL, NULL);
        CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_BRIDGE_CALL);
    }
    
    // Cleanup batch data
    for (int i = 0; i < LARGE_BATCH_SIZE; i++) {
        free(messages[i]);
        free(passwords[i]);
    }
    free(messages);
    free(passwords);
    
    printf("Large batch operations stress test passed\n");
    return 0;
}

int main() {
    printf("\n=== Memory Regression Test Suite - Stress Testing ===\n");
    printf("Testing repeated operations for memory stability\n\n");
    
    int total_failed = 0;
    
    total_failed += run_isolated_test(test_repeated_encryption_operations, "Repeated Encryption");
    total_failed += run_isolated_test(test_repeated_signing_operations, "Repeated Signing");
    total_failed += run_isolated_test(test_repeated_key_generation, "Repeated Key Generation");
    total_failed += run_isolated_test(test_rapid_sequential_operations, "Rapid Sequential Operations");
    total_failed += run_isolated_test(test_mixed_operation_patterns, "Mixed Operation Patterns");
    total_failed += run_isolated_test(test_size_validation_under_stress, "Size Validation Under Stress");
    total_failed += run_isolated_test(test_large_batch_operations, "Large Batch Operations");
    
    printf("\n=== Stress Testing Summary ===\n");
    printf("Tests run: %d, Tests failed: %d\n", get_tests_run(), get_tests_failed());
    
    if (total_failed > 0) {
        printf(COLOR_RED "FAILED: %d test(s) failed\n" COLOR_RESET, total_failed);
        return 1;
    }    printf(COLOR_GREEN "SUCCESS: All stress tests passed\n" COLOR_RESET);
    return 0;
}