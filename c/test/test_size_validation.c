#include "test_framework_isolated.h"
#include "test_isolation_manager.h"
#include "openpgp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test size validation for message encryption */
int test_message_size_validation(void) {
    /* Create a message that exceeds MAX_MESSAGE_SIZE (2KB) */
    size_t large_size = 3 * 1024; /* 3KB message */
    char *large_message = malloc(large_size + 1);
    TEST_ASSERT_NOT_NULL(large_message);
    
    /* Fill with test data */
    memset(large_message, 'A', large_size);
    large_message[large_size] = '\0';
    
    /* Try to encrypt - should fail with size limit error */
    openpgp_result_t result = openpgp_encrypt_symmetric(large_message, "test_pass", NULL, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_SIZE_LIMIT, result.error);
    TEST_ASSERT_NOT_NULL(result.error_message);
    
    free(large_message);
    if (result.error_message) {
        free(result.error_message);
    }
    
    return 0;
}/* Test size validation for signature data */
int test_signature_data_size_validation(void) {
    /* Create data that exceeds MAX_SIGNATURE_DATA_SIZE (3KB) */
    size_t large_size = 4 * 1024; /* 4KB data */
    char *large_data = malloc(large_size + 1);
    TEST_ASSERT_NOT_NULL(large_data);
    
    /* Fill with test data */
    memset(large_data, 'B', large_size);
    large_data[large_size] = '\0';
    
    /* Try to sign - should fail with size limit error */
    char *dummy_key = "dummy_private_key";
    openpgp_result_t result = openpgp_sign(large_data, dummy_key, "pass", NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_SIZE_LIMIT, result.error);
    TEST_ASSERT_NOT_NULL(result.error_message);
    
    free(large_data);
    if (result.error_message) {
        free(result.error_message);
    }
    
    return 0;
}

/* Main function to test size validation */
int main(void) {
    printf("Testing Size Validation System\n");
    printf("===============================\n");
    
    /* Initialize isolation system */
    test_isolation_init();
    
    int failures = 0;
    
    /* Run size validation tests */
    printf("\n--- Testing size validation ---\n");
    if (RUN_ISOLATED_TEST(message_size_validation) != 0) failures++;
    if (RUN_ISOLATED_TEST(signature_data_size_validation) != 0) failures++;
    
    /* Cleanup isolation system */
    test_isolation_cleanup();
    
    printf("\n===============================\n");
    printf("Tests run: %d, Tests failed: %d, Major tests run: %d, Major tests failed: %d\n",
           get_tests_run(), get_tests_failed(), get_major_tests_run(), get_major_tests_failed());
    
    if (failures == 0) {
        printf("All size validation tests PASSED!\n");
        return 0;
    } else {
        printf("%d size validation tests FAILED!\n", failures);
        return 1;
    }
}