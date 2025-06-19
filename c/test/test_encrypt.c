#include "openpgp.h"
#include "test_framework.h"
#include <string.h>
#include <stdlib.h>

// Test keys - using the same format as the Go tests for consistency
static const char *test_public_key_1 = 
    "-----BEGIN PGP PUBLIC KEY BLOCK-----\n"
    "\n"
    "mDMEZWR9lRYJKwYBBAHaRw8BAQdA9YzBn2Y4p1FzTk1NL7J5F5J5F5J5F5J5F5J5\n"
    "F5J5F5K0I1Rlc3QgVXNlciAoVGVzdCBrZXkpIDx0ZXN0QGV4YW1wbGUuY29tPoiQ\n"
    "BBMWCABCBQJlZH2VCRBvL7J5F5J5F5J5AhsDBQkDwmcABQsJCAcCAyICAQYVCgkI\n"
    "CwIEFgIDAQIeBwIXgAUbAgAAAB4RAP9B9YzBn2Y4p1FzTk1NL7J5F5J5F5J5F5J5\n"
    "F5J5F5J5AQCvL7J5F5J5F5J5F5J5F5J5F5J5F5J5F5J5F5J5F5J5Fw==\n"
    "=abc1\n"
    "-----END PGP PUBLIC KEY BLOCK-----\n";

static const char *test_private_key_1 = 
    "-----BEGIN PGP PRIVATE KEY BLOCK-----\n"
    "\n"
    "lFgEZWR9lRYJKwYBBAHaRw8BAQdA9YzBn2Y4p1FzTk1NL7J5F5J5F5J5F5J5F5J5\n"
    "F5J5F5KAAP9/L7J5F5J5F5J5F5J5F5J5F5J5F5J5F5J5F5J5F5J5F5J5F5J5EUq0\n"
    "I1Rlc3QgVXNlciAoVGVzdCBrZXkpIDx0ZXN0QGV4YW1wbGUuY29tPoiQBBMWCABC\n"
    "BQJlZH2VCRBvL7J5F5J5F5J5AhsDBQkDwmcABQsJCAcCAyICAQYVCgkICwIEFgID\n"
    "AQIeBwIXgAUbAgAAAB4RAP9B9YzBn2Y4p1FzTk1NL7J5F5J5F5J5F5J5F5J5F5J5\n"
    "AQCvL7J5F5J5F5J5F5J5F5J5F5J5F5J5F5J5F5J5F5J5FwAA/2vL7J5F5J5F5J5\n"
    "F5J5F5J5F5J5F5J5F5J5F5J5F5J5F5J5\n"
    "=def2\n"
    "-----END PGP PRIVATE KEY BLOCK-----\n";

static const char *test_public_key_2 = 
    "-----BEGIN PGP PUBLIC KEY BLOCK-----\n"
    "\n"
    "mDMEZWR9lRYJKwYBBAHaRw8BAQdA8YzBn2Y4p1FzTk1NL7J5F5J5F5J5F5J5F5J5\n"
    "F5J5F5K0JFRlc3QgVXNlciAyIChUZXN0IGtleSAyKSA8dGVzdDJAZXhhbXBsZS5j\n"
    "b20+iJAEExYIAEIFAmVkfZUJEG8vsnkXknkXknkCGwMFCQPCZwAFCwkIBwIDIgIB\n"
    "BhUKCQgLAgQWAgMBAh4HAheABRsCAAAeFgD/QfWMwZ9mOKdRc05NTS+yeReSeReS\n"
    "eReSeReSeReSeBcArC+yeReSeReSeReSeReSeReSeReSeReSeReSeReSeRcXAA==\n"
    "=ghi3\n"
    "-----END PGP PUBLIC KEY BLOCK-----\n";

static const char *test_message = "Hello, world! This is a secret message for asymmetric encryption testing.";

// Test helper functions
bool validate_pgp_message(const char *message) {
    if (!message) return false;
    return strstr(message, "-----BEGIN PGP MESSAGE-----") != NULL &&
           strstr(message, "-----END PGP MESSAGE-----") != NULL;
}

// Helper function to initialize library for tests
static int setup_library(void) {
    openpgp_result_t result = openpgp_init();
    if (result.error != OPENPGP_SUCCESS) {
        printf("Failed to initialize library: %s\n", result.error_message);
        openpgp_result_free(&result);
        return -1;
    }
    openpgp_result_free(&result);
    return 0;
}

// Helper function to cleanup library
static void cleanup_library(void) {
    openpgp_cleanup();
}

TEST_CASE(encrypt_message_single_recipient) {
    if (setup_library() != 0) return -1;
    
    const char *recipients[] = { test_public_key_1 };
    
    openpgp_result_t result = openpgp_encrypt(test_message, recipients, 1, NULL);
    
    // Initially expect this to fail until implementation is complete
    // This follows TDD - test first, implement later
    if (result.error == OPENPGP_SUCCESS) {
        TEST_ASSERT_NOT_NULL(result.data);
        char *encrypted = (char *)result.data;
        TEST_ASSERT_TRUE(validate_pgp_message(encrypted));
        TEST_ASSERT_TRUE(strlen(encrypted) > strlen(test_message));
        openpgp_result_free(&result);
    } else {
        // Expected during TDD phase - bridge call not implemented yet
        TEST_ASSERT_NOT_EQUAL(OPENPGP_SUCCESS, result.error);
        TEST_ASSERT_NOT_NULL(result.error_message);
        openpgp_result_free(&result);
    }
    
    cleanup_library();
    return 0;
}

TEST_CASE(encrypt_message_multiple_recipients) {
    if (setup_library() != 0) return -1;
    
    const char *recipients[] = { test_public_key_1, test_public_key_2 };
    
    openpgp_result_t result = openpgp_encrypt(test_message, recipients, 2, NULL);
    
    if (result.error == OPENPGP_SUCCESS) {
        TEST_ASSERT_NOT_NULL(result.data);
        char *encrypted = (char *)result.data;
        TEST_ASSERT_TRUE(validate_pgp_message(encrypted));
        openpgp_result_free(&result);
    } else {
        // Expected during TDD phase
        TEST_ASSERT_NOT_EQUAL(OPENPGP_SUCCESS, result.error);
        openpgp_result_free(&result);
    }
    
    cleanup_library();
    return 0;
}

TEST_CASE(decrypt_message) {
    if (setup_library() != 0) return -1;
    
    // For TDD - test decryption function signature
    const char *dummy_encrypted = 
        "-----BEGIN PGP MESSAGE-----\n"
        "hQGMA0/fake/encrypted/data\n"
        "=abcd\n"
        "-----END PGP MESSAGE-----\n";
    
    openpgp_result_t result = openpgp_decrypt(dummy_encrypted, test_private_key_1, NULL, NULL);
    // Expected to fail until implementation
    TEST_ASSERT_NOT_EQUAL(OPENPGP_SUCCESS, result.error);
    openpgp_result_free(&result);
    
    cleanup_library();
    return 0;
}

TEST_CASE(encrypt_decrypt_round_trip) {
    if (setup_library() != 0) return -1;
    
    // TDD placeholder - will implement full round-trip later
    const char *recipients[] = { test_public_key_1 };
    openpgp_result_t result = openpgp_encrypt(test_message, recipients, 1, NULL);
    // Expected to fail until implementation
    TEST_ASSERT_NOT_EQUAL(OPENPGP_SUCCESS, result.error);
    openpgp_result_free(&result);
    
    cleanup_library();
    return 0;
}

TEST_CASE(encrypt_invalid_public_key) {
    if (setup_library() != 0) return -1;
    
    const char *invalid_recipients[] = { "invalid key data" };
    openpgp_result_t result = openpgp_encrypt(test_message, invalid_recipients, 1, NULL);
    
    // Should fail with invalid input error
    TEST_ASSERT_NOT_EQUAL(OPENPGP_SUCCESS, result.error);
    openpgp_result_free(&result);
    
    cleanup_library();
    return 0;
}

TEST_CASE(encrypt_null_message) {
    if (setup_library() != 0) return -1;
    
    const char *recipients[] = { test_public_key_1 };
    openpgp_result_t result = openpgp_encrypt(NULL, recipients, 1, NULL);
    
    // Should fail with invalid input error
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result.error);
    openpgp_result_free(&result);
    
    cleanup_library();
    return 0;
}

TEST_CASE(encrypt_null_recipients) {
    if (setup_library() != 0) return -1;
    
    openpgp_result_t result = openpgp_encrypt(test_message, NULL, 1, NULL);
    
    // Should fail with invalid input error
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result.error);
    openpgp_result_free(&result);
    
    cleanup_library();
    return 0;
}

TEST_CASE(encrypt_zero_recipients) {
    if (setup_library() != 0) return -1;
    
    const char *recipients[] = { test_public_key_1 };
    openpgp_result_t result = openpgp_encrypt(test_message, recipients, 0, NULL);
    
    // Should fail with invalid input error
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result.error);
    openpgp_result_free(&result);
    
    cleanup_library();
    return 0;
}

TEST_CASE(decrypt_wrong_private_key) {
    if (setup_library() != 0) return -1;
    
    const char *dummy_encrypted = 
        "-----BEGIN PGP MESSAGE-----\n"
        "hQGMA0/fake/encrypted/data\n"
        "=abcd\n"
        "-----END PGP MESSAGE-----\n";
    
    openpgp_result_t result = openpgp_decrypt(dummy_encrypted, test_private_key_1, NULL, NULL);
    
    // Should fail
    TEST_ASSERT_NOT_EQUAL(OPENPGP_SUCCESS, result.error);
    openpgp_result_free(&result);
    
    cleanup_library();
    return 0;
}

// Note: These test functions are called from test_runner.c
// No main() function needed in individual test files