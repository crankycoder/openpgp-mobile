#include "test_framework.h"
#include "openpgp.h"

TEST_CASE(generate_key_basic) {
    openpgp_result_t init_result = openpgp_init();
    
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("      Skipping - bridge not available: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 0;
    }
    openpgp_result_free(&init_result);
    
    /* Test basic key generation */
    openpgp_result_t result = openpgp_generate_key("Test User", "test@example.com", "testpass");
    
    /* Now that FlatBuffers are working, let's see what happens */
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, result.error);
    TEST_ASSERT_NULL(result.error_message);
    /* TODO: Verify the key pair data */
    
    openpgp_result_free(&result);
    openpgp_cleanup();
    return 0;
}

TEST_CASE(generate_key_with_options) {
    openpgp_result_t init_result = openpgp_init();
    
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("      Skipping - bridge not available: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 0;
    }
    openpgp_result_free(&init_result);
    
    /* Test key generation with custom options */
    openpgp_options_t options;
    openpgp_options_init_default(&options);
    options.name = "Test User";
    options.email = "test@example.com";
    options.passphrase = "testpass";
    options.comment = "Test Key";
    options.key_options.algorithm = OPENPGP_ALGORITHM_RSA;
    options.key_options.rsa_bits = 2048;
    options.key_options.hash = OPENPGP_HASH_SHA256;
    
    openpgp_result_t result = openpgp_generate_key_with_options(&options);
    
    /* Now that FlatBuffers work, this should succeed */
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, result.error);
    TEST_ASSERT_NULL(result.error_message);
    
    openpgp_result_free(&result);
    openpgp_cleanup();
    return 0;
}

TEST_CASE(generate_key_input_validation) {
    openpgp_result_t init_result = openpgp_init();
    
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("      Skipping - bridge not available: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 0;
    }
    openpgp_result_free(&init_result);
    
    /* Test input validation */
    openpgp_result_t result = openpgp_generate_key_with_options(NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result.error);
    TEST_ASSERT_NOT_NULL(result.error_message);
    openpgp_result_free(&result);
    
    openpgp_cleanup();
    return 0;
}

TEST_CASE(generate_key_without_init) {
    /* Test calling without initialization */
    openpgp_result_t result = openpgp_generate_key("Test", "test@example.com", NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED, result.error);
    TEST_ASSERT_NOT_NULL(result.error_message);
    openpgp_result_free(&result);
    
    return 0;
}