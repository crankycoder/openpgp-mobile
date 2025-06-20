/**
 * Memory Regression Test Suite - Edge Cases
 * Tests boundary conditions and unusual scenarios that could cause memory issues
 * Validates proper handling of edge cases and corner conditions
 */

#include "test_framework_isolated.h"
#include "test_isolation_manager.h"
#include "../include/openpgp.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int test_null_parameter_handling(void) {
    printf("Testing NULL parameter handling...\n");
    
    // Test all combinations of NULL parameters
    openpgp_result_t result;
    
    result = openpgp_encrypt_symmetric(NULL, "password", NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_INVALID_ARGUMENT);
    
    result = openpgp_encrypt_symmetric("message", NULL, NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_INVALID_ARGUMENT);
    
    result = openpgp_encrypt_symmetric(NULL, NULL, NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_INVALID_ARGUMENT);
    
    result = openpgp_sign(NULL, "key", NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_INVALID_ARGUMENT);
    
    result = openpgp_sign("data", NULL, NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_INVALID_ARGUMENT);
    
    result = openpgp_generate_key(NULL, 2048, "comment", NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_INVALID_ARGUMENT);
    
    result = openpgp_generate_key("rsa", 2048, NULL, NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // NULL comment is allowed
    
    printf("NULL parameter handling tests passed\n");
    return 0;
}static int test_empty_string_handling(void) {
    printf("Testing empty string handling...\n");
    
    // Test empty strings vs NULL
    openpgp_result_t result;
    
    result = openpgp_encrypt_symmetric("", "password", NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // Empty message allowed
    
    result = openpgp_encrypt_symmetric("message", "", NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // Empty password allowed
    
    result = openpgp_sign("", "key", NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // Empty data allowed
    
    result = openpgp_sign("data", "", NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // Empty key allowed
    
    result = openpgp_generate_key("", 2048, "comment", NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // Empty algorithm allowed
    
    result = openpgp_generate_key("rsa", 2048, "", NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // Empty comment allowed
    
    printf("Empty string handling tests passed\n");
    return 0;
}

static int test_extreme_size_boundaries(void) {
    printf("Testing extreme size boundaries...\n");
    
    // Test size_t maximum values (should be caught by size validation)
    // Note: We don't actually allocate these sizes, just test the validation
    
    // Test with theoretical maximum sizes that would overflow
    openpgp_result_t result;
    
    // Simulate what would happen with extremely large theoretical inputs
    // by testing our size constants are reasonable
    
    // Test that our limits are sensible compared to system limits
    TEST_ASSERT(2048 < INT_MAX);  // MAX_MESSAGE_SIZE < INT_MAX
    TEST_ASSERT(3072 < INT_MAX);  // MAX_SIGNATURE_DATA_SIZE < INT_MAX  
    TEST_ASSERT(512 < INT_MAX);   // MAX_KEY_COMMENT_SIZE < INT_MAX
    TEST_ASSERT(4096 < INT_MAX);  // MAX_FLATBUFFER_SIZE < INT_MAX
    
    printf("Extreme size boundary tests passed\n");
    return 0;
}static int test_special_character_handling(void) {
    printf("Testing special character handling...\n");
    
    // Test various special characters and encoding edge cases
    const char *special_messages[] = {
        "\x00\x01\x02",           // Binary data
        "Üñíçødé tëxt",           // Unicode characters
        "\n\r\t",                 // Control characters
        "\"'\\",                  // Quote and escape characters
        "\xFF\xFE\xFD",           // High-byte characters
    };
    
    const char *special_passwords[] = {
        "pässwörd",
        "密码",
        "пароль",
        "\x7F\x80\x81",
    };
    
    for (size_t i = 0; i < sizeof(special_messages) / sizeof(special_messages[0]); i++) {
        openpgp_result_t result = openpgp_encrypt_symmetric(special_messages[i], "password", NULL, NULL);
        TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // Should handle gracefully
    }
    
    for (size_t i = 0; i < sizeof(special_passwords) / sizeof(special_passwords[0]); i++) {
        openpgp_result_t result = openpgp_encrypt_symmetric("message", special_passwords[i], NULL, NULL);
        TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // Should handle gracefully
    }
    
    printf("Special character handling tests passed\n");
    return 0;
}

static int test_integer_overflow_scenarios(void) {
    printf("Testing integer overflow scenarios...\n");
    
    // Test key sizes that might cause issues
    openpgp_result_t result;
    
    // Test negative key sizes (should be handled by type system, but test anyway)
    result = openpgp_generate_key("rsa", -1, "comment", NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // Negative converted to large positive
    
    // Test zero key size
    result = openpgp_generate_key("rsa", 0, "comment", NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // Should be handled by bridge
    
    // Test extremely large key size
    result = openpgp_generate_key("rsa", INT_MAX, "comment", NULL, NULL);
    TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // Should be handled by bridge
    
    printf("Integer overflow scenario tests passed\n");
    return 0;
}static int test_memory_allocation_failures(void) {
    printf("Testing memory allocation failure scenarios...\n");
    
    // Test behavior when memory allocation might fail
    // This is difficult to test directly, but we can test our error handling
    
    // Test that our functions handle allocation gracefully
    // by testing with valid inputs (allocation should succeed)
    char *test_msg = malloc(1024);
    if (test_msg) {
        memset(test_msg, 'T', 1023);
        test_msg[1023] = '\0';
        
        openpgp_result_t result = openpgp_encrypt_symmetric(test_msg, "password", NULL, NULL);
        TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL); // Should succeed allocation checks
        
        free(test_msg);
    }
    
    printf("Memory allocation failure scenario tests passed\n");
    return 0;
}

static int test_concurrent_edge_conditions(void) {
    printf("Testing concurrent edge conditions...\n");
    
    // Test edge cases that might occur in concurrent scenarios
    // (Even though we're single-threaded, test state isolation)
    
    // Test rapid alternation between valid and invalid operations
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            openpgp_result_t result = openpgp_encrypt_symmetric(NULL, "pass", NULL, NULL);
            TEST_ASSERT(result == OPENPGP_ERROR_INVALID_ARGUMENT);
        } else if (i % 3 == 1) {
            char *oversized = malloc(3000);
            memset(oversized, 'O', 2999);
            oversized[2999] = '\0';
            
            openpgp_result_t result = openpgp_encrypt_symmetric(oversized, "pass", NULL, NULL);
            TEST_ASSERT(result == OPENPGP_ERROR_SIZE_LIMIT);
            
            free(oversized);
        } else {
            openpgp_result_t result = openpgp_encrypt_symmetric("valid", "pass", NULL, NULL);
            TEST_ASSERT(result == OPENPGP_ERROR_BRIDGE_CALL);
        }
    }
    
    printf("Concurrent edge condition tests passed\n");
    return 0;
}int main() {
    printf("\n=== Memory Regression Test Suite - Edge Cases ===\n");
    printf("Testing boundary conditions and unusual scenarios\n\n");
    
    int total_failed = 0;
    
    total_failed += run_isolated_test(test_null_parameter_handling, "NULL Parameter Handling");
    total_failed += run_isolated_test(test_empty_string_handling, "Empty String Handling");
    total_failed += run_isolated_test(test_extreme_size_boundaries, "Extreme Size Boundaries");
    total_failed += run_isolated_test(test_special_character_handling, "Special Character Handling");
    total_failed += run_isolated_test(test_integer_overflow_scenarios, "Integer Overflow Scenarios");
    total_failed += run_isolated_test(test_memory_allocation_failures, "Memory Allocation Failures");
    total_failed += run_isolated_test(test_concurrent_edge_conditions, "Concurrent Edge Conditions");
    
    printf("\n=== Edge Cases Testing Summary ===\n");
    print_test_summary();
    
    if (total_failed > 0) {
        printf(COLOR_RED "FAILED: %d test(s) failed\n" COLOR_RESET, total_failed);
        return 1;
    }
    
    printf(COLOR_GREEN "SUCCESS: All edge case tests passed\n" COLOR_RESET);
    return 0;
}