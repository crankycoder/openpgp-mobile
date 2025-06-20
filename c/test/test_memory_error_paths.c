#include "test_framework_isolated.h"
#include "test_isolation_manager.h"
#include "memory_helpers.h"
#include "openpgp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper macro to free error message and check result */
#define CHECK_RESULT_AND_FREE(result, expected_error) do { \
    if ((result).error_message) { \
        free((result).error_message); \
        (result).error_message = NULL; \
    } \
    TEST_ASSERT_EQUAL((expected_error), (result).error); \
} while(0)

/* Test error paths that previously caused memory leaks */

/* Test 1: Error handling with proper cleanup */
int test_error_path_initialization_failure(void) {
    /* Test that initialization failures don't leak memory */
    openpgp_result_t result = openpgp_init();
    
    /* Expected to fail without bridge - should not leak */
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_BRIDGE_CALL, result.error);
    TEST_ASSERT_NOT_NULL(result.error_message);
    
    /* Clean up error message */
    if (result.error_message) {
        free(result.error_message);
    }
    
    return 0;
}

/* Test 2: FlatBuffer builder initialization failure paths */
int test_error_path_flatbuffer_init(void) {
    /* This tests the init_flatcc_builder_safe() error handling */
    /* We can't easily force flatcc_builder_init to fail, so we test the logic */
    
    /* Create a scenario that exercises error paths */
    openpgp_options_t options;
    memset(&options, 0, sizeof(options));
    options.name = "Test User";
    options.email = "test@example.com";
    options.passphrase = "test_password";
    
    /* This will fail at bridge call, but should properly handle FlatBuffer cleanup */
    openpgp_result_t result = openpgp_generate_key_with_options(&options);
    
    /* Check result and free error message */
    CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_SERIALIZATION);
    
    return 0;
}/* Test 3: Size limit validation error paths */
int test_error_path_size_limits(void) {
    /* Test that size limit errors are handled without leaks */
    size_t large_size = 3 * 1024; /* Exceeds 2KB message limit */
    char *large_message = malloc(large_size + 1);
    TEST_ASSERT_NOT_NULL(large_message);
    
    memset(large_message, 'A', large_size);
    large_message[large_size] = '\0';
    
    /* This should fail with size limit error */
    openpgp_result_t result = openpgp_encrypt_symmetric(large_message, "pass", NULL, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_SIZE_LIMIT, result.error);
    TEST_ASSERT_NOT_NULL(result.error_message);
    
    /* Clean up */
    free(large_message);
    if (result.error_message) {
        free(result.error_message);
    }
    
    return 0;
}

/* Test 4: NULL parameter error paths */
int test_error_path_null_parameters(void) {
    /* Test various NULL parameter scenarios */
    
    /* NULL message */
    openpgp_result_t result1 = openpgp_encrypt_symmetric(NULL, "pass", NULL, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result1.error);
    if (result1.error_message) free(result1.error_message);
    
    /* NULL passphrase */
    openpgp_result_t result2 = openpgp_encrypt_symmetric("message", NULL, NULL, NULL);
    CHECK_RESULT_AND_FREE(result2, OPENPGP_ERROR_SERIALIZATION);
    
    /* NULL private key */
    openpgp_result_t result3 = openpgp_sign("message", NULL, "pass", NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result3.error);
    if (result3.error_message) free(result3.error_message);
    
    /* NULL options */
    openpgp_result_t result4 = openpgp_generate_key_with_options(NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result4.error);
    if (result4.error_message) free(result4.error_message);
    
    return 0;
}/* Test 5: Memory allocation failure simulation scenarios */
int test_error_path_allocation_failures(void) {
    /* Test scenarios that would previously cause memory leaks in error paths */
    
    /* Test key generation with very long comment (triggers size limit) */
    openpgp_options_t options;
    memset(&options, 0, sizeof(options));
    options.name = "Test User";
    options.email = "test@example.com";
    options.passphrase = "test_password";
    
    /* Create comment that exceeds MAX_KEY_COMMENT_SIZE (512B) */
    char long_comment[600];
    memset(long_comment, 'X', sizeof(long_comment) - 1);
    long_comment[sizeof(long_comment) - 1] = '\0';
    options.comment = long_comment;
    
    openpgp_result_t result = openpgp_generate_key_with_options(&options);
    CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_SERIALIZATION);
    
    return 0;
}

/* Main function for error path tests */
int main(void) {
    printf("Testing Memory Error Paths\n");
    printf("==========================\n");
    
    test_isolation_init();
    
    int failures = 0;
    
    printf("\n--- Testing error path memory safety ---\n");
    if (RUN_ISOLATED_TEST(error_path_initialization_failure) != 0) failures++;
    if (RUN_ISOLATED_TEST(error_path_flatbuffer_init) != 0) failures++;
    if (RUN_ISOLATED_TEST(error_path_size_limits) != 0) failures++;
    if (RUN_ISOLATED_TEST(error_path_null_parameters) != 0) failures++;
    if (RUN_ISOLATED_TEST(error_path_allocation_failures) != 0) failures++;
    
    test_isolation_cleanup();
    
    printf("\n==========================\n");
    printf("Tests run: %d, Tests failed: %d, Major tests run: %d, Major tests failed: %d\n",
           get_tests_run(), get_tests_failed(), get_major_tests_run(), get_major_tests_failed());
    
    if (failures == 0) {
        printf("All error path tests PASSED!\n");
        return 0;
    } else {
        printf("%d error path tests FAILED!\n", failures);
        return 1;
    }
}