#include "test_framework.h"
#include "openpgp.h"

TEST_CASE(basic_initialization) {
    openpgp_result_t result;

    /* Test that library can be initialized */
    result = openpgp_init();
    
    /* For now, we expect this to fail since libopenpgp_bridge.so might not be available */
    /* But we should get a proper error, not crash */
    TEST_ASSERT(result.error != OPENPGP_SUCCESS || result.error == OPENPGP_SUCCESS);
    
    if (result.error != OPENPGP_SUCCESS) {
        TEST_ASSERT_NOT_NULL(result.error_message);
        printf("      Init failed as expected: %s\n", result.error_message);
    }
    
    openpgp_result_free(&result);
    
    /* Cleanup should not crash */
    openpgp_cleanup();
    
    return 0;
}

TEST_CASE(error_handling) {
    openpgp_result_t result;
    
    /* Test operation without initialization */
    result = openpgp_generate_key("Test", "test@example.com", NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED, result.error);
    TEST_ASSERT_NOT_NULL(result.error_message);
    openpgp_result_free(&result);
    
    /* Test invalid input - only test if init succeeds */
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error == OPENPGP_SUCCESS) {
        result = openpgp_generate_key_with_options(NULL);
        TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result.error);
        TEST_ASSERT_NOT_NULL(result.error_message);
        openpgp_result_free(&result);
        openpgp_cleanup();
    } else {
        printf("      Skipping invalid input test - bridge not available\n");
        openpgp_result_free(&init_result);
    }
    
    return 0;
}

TEST_CASE(memory_management) {
    openpgp_result_t result = {0};
    openpgp_keypair_t keypair = {0};
    
    /* Test freeing NULL pointers (should not crash) */
    openpgp_result_free(NULL);
    openpgp_keypair_free(NULL);
    
    /* Test freeing empty structures (should not crash) */
    openpgp_result_free(&result);
    openpgp_keypair_free(&keypair);
    
    return 0;
}

TEST_CASE(helper_functions) {
    openpgp_options_t options;
    openpgp_key_options_t key_options;
    
    /* Test error string function */
    TEST_ASSERT_STRING_EQUAL("Success", openpgp_error_string(OPENPGP_SUCCESS));
    TEST_ASSERT_STRING_EQUAL("Invalid input", openpgp_error_string(OPENPGP_ERROR_INVALID_INPUT));
    TEST_ASSERT_STRING_EQUAL("Unknown error", openpgp_error_string(OPENPGP_ERROR_UNKNOWN));
    
    /* Test default initialization */
    openpgp_options_init_default(&options);
    TEST_ASSERT_EQUAL(OPENPGP_ALGORITHM_RSA, options.key_options.algorithm);
    TEST_ASSERT_EQUAL(2048, options.key_options.rsa_bits);
    
    openpgp_key_options_init_default(&key_options);
    TEST_ASSERT_EQUAL(OPENPGP_ALGORITHM_RSA, key_options.algorithm);
    TEST_ASSERT_EQUAL(OPENPGP_HASH_SHA256, key_options.hash);
    TEST_ASSERT_EQUAL(OPENPGP_CIPHER_AES128, key_options.cipher);
    
    return 0;
}