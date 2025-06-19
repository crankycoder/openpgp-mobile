#include "test_framework.h"
#include "openpgp.h"
#include <string.h>
#include <unistd.h>

/* Test data */
static const char *TEST_MESSAGE = "Hello, world! This is a test message for symmetric encryption.";
static const char *TEST_PASSPHRASE = "test_passphrase_123";
static const char *TEST_BINARY_DATA = "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A";
static const size_t TEST_BINARY_SIZE = 10;

TEST_CASE(symmetric_encrypt_decrypt_message) {
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("      Skipping test - bridge not available: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 0;
    }
    openpgp_result_free(&init_result);

    /* Test basic symmetric encryption */
    openpgp_result_t encrypt_result = openpgp_encrypt_symmetric(
        TEST_MESSAGE, TEST_PASSPHRASE, NULL, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, encrypt_result.error);
    TEST_ASSERT_NOT_NULL(encrypt_result.data);
    TEST_ASSERT(encrypt_result.data_size > 0);
    
    char *encrypted_message = (char *)encrypt_result.data;
    TEST_ASSERT_NOT_NULL(encrypted_message);
    TEST_ASSERT(strlen(encrypted_message) > strlen(TEST_MESSAGE));
    
    /* Test symmetric decryption */
    openpgp_result_t decrypt_result = openpgp_decrypt_symmetric(
        encrypted_message, TEST_PASSPHRASE, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, decrypt_result.error);
    TEST_ASSERT_NOT_NULL(decrypt_result.data);
    
    char *decrypted_message = (char *)decrypt_result.data;
    TEST_ASSERT_STRING_EQUAL(TEST_MESSAGE, decrypted_message);
    
    openpgp_result_free(&encrypt_result);
    openpgp_result_free(&decrypt_result);
    openpgp_cleanup();
    
    return 0;
}

TEST_CASE(symmetric_encrypt_decrypt_with_file_hints) {
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("      Skipping test - bridge not available: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 0;
    }
    openpgp_result_free(&init_result);

    /* Setup file hints */
    openpgp_file_hints_t file_hints = {
        .is_binary = false,
        .file_name = "test.txt",
        .mod_time = "2023-01-01T00:00:00Z"
    };
    
    /* Test encryption with file hints */
    openpgp_result_t encrypt_result = openpgp_encrypt_symmetric(
        TEST_MESSAGE, TEST_PASSPHRASE, &file_hints, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, encrypt_result.error);
    TEST_ASSERT_NOT_NULL(encrypt_result.data);
    
    char *encrypted_message = (char *)encrypt_result.data;
    
    /* Test decryption */
    openpgp_result_t decrypt_result = openpgp_decrypt_symmetric(
        encrypted_message, TEST_PASSPHRASE, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, decrypt_result.error);
    TEST_ASSERT_NOT_NULL(decrypt_result.data);
    
    char *decrypted_message = (char *)decrypt_result.data;
    TEST_ASSERT_STRING_EQUAL(TEST_MESSAGE, decrypted_message);
    
    openpgp_result_free(&encrypt_result);
    openpgp_result_free(&decrypt_result);
    openpgp_cleanup();
    
    return 0;
}

TEST_CASE(symmetric_encrypt_decrypt_with_options) {
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("      Skipping test - bridge not available: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 0;
    }
    openpgp_result_free(&init_result);

    /* Setup key options */
    openpgp_key_options_t options;
    openpgp_key_options_init_default(&options);
    options.cipher = OPENPGP_CIPHER_AES256;
    options.hash = OPENPGP_HASH_SHA256;
    options.compression = OPENPGP_COMPRESSION_ZLIB;
    
    /* Test encryption with options */
    openpgp_result_t encrypt_result = openpgp_encrypt_symmetric(
        TEST_MESSAGE, TEST_PASSPHRASE, NULL, &options);
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, encrypt_result.error);
    TEST_ASSERT_NOT_NULL(encrypt_result.data);
    
    char *encrypted_message = (char *)encrypt_result.data;
    
    /* Test decryption with options */
    openpgp_result_t decrypt_result = openpgp_decrypt_symmetric(
        encrypted_message, TEST_PASSPHRASE, &options);
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, decrypt_result.error);
    TEST_ASSERT_NOT_NULL(decrypt_result.data);
    
    char *decrypted_message = (char *)decrypt_result.data;
    TEST_ASSERT_STRING_EQUAL(TEST_MESSAGE, decrypted_message);
    
    openpgp_result_free(&encrypt_result);
    openpgp_result_free(&decrypt_result);
    openpgp_cleanup();
    
    return 0;
}

TEST_CASE(symmetric_encrypt_decrypt_bytes) {
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("      Skipping test - bridge not available: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 0;
    }
    openpgp_result_free(&init_result);

    /* Test bytes encryption */
    openpgp_result_t encrypt_result = openpgp_encrypt_symmetric_bytes(
        (const uint8_t *)TEST_BINARY_DATA, TEST_BINARY_SIZE, TEST_PASSPHRASE, NULL, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, encrypt_result.error);
    TEST_ASSERT_NOT_NULL(encrypt_result.data);
    TEST_ASSERT(encrypt_result.data_size > TEST_BINARY_SIZE);
    
    /* Test bytes decryption */
    openpgp_result_t decrypt_result = openpgp_decrypt_symmetric_bytes(
        (const uint8_t *)encrypt_result.data, encrypt_result.data_size, TEST_PASSPHRASE, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, decrypt_result.error);
    TEST_ASSERT_NOT_NULL(decrypt_result.data);
    TEST_ASSERT_EQUAL(TEST_BINARY_SIZE, decrypt_result.data_size);
    
    /* Compare decrypted data */
    TEST_ASSERT(memcmp(TEST_BINARY_DATA, decrypt_result.data, TEST_BINARY_SIZE) == 0);
    
    openpgp_result_free(&encrypt_result);
    openpgp_result_free(&decrypt_result);
    openpgp_cleanup();
    
    return 0;
}

TEST_CASE(symmetric_encrypt_decrypt_file) {
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("      Skipping test - bridge not available: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 0;
    }
    openpgp_result_free(&init_result);

    /* Create test files */
    const char *input_file = "/tmp/test_input.txt";
    const char *encrypted_file = "/tmp/test_encrypted.pgp";
    const char *decrypted_file = "/tmp/test_decrypted.txt";
    
    /* Write test data to input file */
    FILE *fp = fopen(input_file, "w");
    TEST_ASSERT_NOT_NULL(fp);
    fputs(TEST_MESSAGE, fp);
    fclose(fp);
    
    /* Test file encryption */
    openpgp_result_t encrypt_result = openpgp_encrypt_symmetric_file(
        input_file, encrypted_file, TEST_PASSPHRASE, NULL, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, encrypt_result.error);
    
    /* Test file decryption */
    openpgp_result_t decrypt_result = openpgp_decrypt_symmetric_file(
        encrypted_file, decrypted_file, TEST_PASSPHRASE, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, decrypt_result.error);
    
    /* Verify decrypted content */
    fp = fopen(decrypted_file, "r");
    TEST_ASSERT_NOT_NULL(fp);
    char buffer[256];
    char *result = fgets(buffer, sizeof(buffer), fp);
    fclose(fp);
    
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_STRING_EQUAL(TEST_MESSAGE, buffer);
    
    /* Cleanup files */
    unlink(input_file);
    unlink(encrypted_file);
    unlink(decrypted_file);
    
    openpgp_result_free(&encrypt_result);
    openpgp_result_free(&decrypt_result);
    openpgp_cleanup();
    
    return 0;
}

TEST_CASE(symmetric_error_handling) {
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("      Skipping test - bridge not available: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 0;
    }
    openpgp_result_free(&init_result);

    /* Test NULL message */
    openpgp_result_t result = openpgp_encrypt_symmetric(NULL, TEST_PASSPHRASE, NULL, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result.error);
    openpgp_result_free(&result);
    
    /* Test NULL passphrase */
    result = openpgp_encrypt_symmetric(TEST_MESSAGE, NULL, NULL, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result.error);
    openpgp_result_free(&result);
    
    /* Test empty message */
    result = openpgp_encrypt_symmetric("", TEST_PASSPHRASE, NULL, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result.error);
    openpgp_result_free(&result);
    
    /* Test empty passphrase */
    result = openpgp_encrypt_symmetric(TEST_MESSAGE, "", NULL, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result.error);
    openpgp_result_free(&result);
    
    /* Test wrong passphrase for decryption */
    result = openpgp_encrypt_symmetric(TEST_MESSAGE, TEST_PASSPHRASE, NULL, NULL);
    if (result.error == OPENPGP_SUCCESS) {
        char *encrypted = (char *)result.data;
        openpgp_result_t decrypt_result = openpgp_decrypt_symmetric(encrypted, "wrong_passphrase", NULL);
        TEST_ASSERT_EQUAL(OPENPGP_ERROR_DECRYPTION_FAILED, decrypt_result.error);
        openpgp_result_free(&decrypt_result);
    }
    openpgp_result_free(&result);
    
    openpgp_cleanup();
    
    return 0;
}

TEST_CASE(symmetric_uninitialized_library) {
    /* Test operations without initialization */
    openpgp_result_t result = openpgp_encrypt_symmetric(TEST_MESSAGE, TEST_PASSPHRASE, NULL, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED, result.error);
    openpgp_result_free(&result);
    
    result = openpgp_decrypt_symmetric("encrypted_message", TEST_PASSPHRASE, NULL);
    TEST_ASSERT_EQUAL(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED, result.error);
    openpgp_result_free(&result);
    
    return 0;
}