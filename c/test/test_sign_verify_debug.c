/**
 * Debug test harness for signing/verification segfault investigation
 * 
 * This file contains debugging tests to systematically isolate
 * the segmentation fault occurring in signing integration tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test_framework.h"
#include "../include/openpgp.h"

/* Global test counters - declared extern, defined in test_runner.c */
extern int g_tests_run;        /* Total assertions/subtests */
extern int g_tests_failed;     /* Failed assertions/subtests */
extern int g_major_tests_run;  /* Major test functions */
extern int g_major_tests_failed; /* Failed major test functions */

// Debug control flags
#ifndef DEBUG_VERBOSE
#define DEBUG_VERBOSE 1
#endif

#ifndef DEBUG_MEMORY
#define DEBUG_MEMORY 1
#endif

// Debug output macros
#if DEBUG_VERBOSE
#define DEBUG_PRINT(fmt, ...) \
    printf("[DEBUG] %s:%d: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) do {} while(0)
#endif

#if DEBUG_MEMORY
#define DEBUG_MEM(ptr, size) \
    printf("[MEM] %s:%d: ptr=%p, size=%zu\n", __func__, __LINE__, (void*)(ptr), (size_t)(size))
#else
#define DEBUG_MEM(ptr, size) do {} while(0)
#endif

// Test key loading functions (will load from gpg-test-keys directory)
static char* load_test_private_key_no_passphrase(void) {
    const char* key_path = "c/test/gpg-test-keys/test-private-key.asc";
    
    DEBUG_PRINT("Loading private key from %s", key_path);
    
    FILE* file = fopen(key_path, "r");
    if (!file) {
        DEBUG_PRINT("Failed to open key file: %s", key_path);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    DEBUG_MEM(NULL, file_size);
    
    // Allocate buffer
    char* buffer = malloc(file_size + 1);
    if (!buffer) {
        DEBUG_PRINT("Failed to allocate memory for key");
        fclose(file);
        return NULL;
    }
    
    // Read file
    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';
    
    fclose(file);
    
    DEBUG_PRINT("Loaded key, size=%zu bytes", bytes_read);
    DEBUG_MEM(buffer, bytes_read);
    
    return buffer;
}

static char* load_test_public_key_no_passphrase(void) {
    const char* key_path = "c/test/gpg-test-keys/test-public-key.asc";
    
    DEBUG_PRINT("Loading public key from %s", key_path);
    
    FILE* file = fopen(key_path, "r");
    if (!file) {
        DEBUG_PRINT("Failed to open key file: %s", key_path);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    DEBUG_MEM(NULL, file_size);
    
    // Allocate buffer
    char* buffer = malloc(file_size + 1);
    if (!buffer) {
        DEBUG_PRINT("Failed to allocate memory for key");
        fclose(file);
        return NULL;
    }
    
    // Read file
    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';
    
    fclose(file);
    
    DEBUG_PRINT("Loaded key, size=%zu bytes", bytes_read);
    DEBUG_MEM(buffer, bytes_read);
    
    return buffer;
}

/**
 * Test 1: Basic library initialization
 * This tests the absolute minimum - can we initialize and cleanup the library?
 */
TEST_CASE(debug_library_init) {
    DEBUG_PRINT("Testing basic library initialization");
    
    DEBUG_PRINT("Calling openpgp_init()");
    openpgp_result_t result = openpgp_init();
    DEBUG_PRINT("Init result: error=%d", result.error);
    
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, result.error);
    
    DEBUG_PRINT("Calling openpgp_cleanup()");
    openpgp_cleanup();
    DEBUG_PRINT("Library cleanup complete");
    
    return 0;
}

/**
 * Test 2: Key loading verification
 * This tests that we can load fixture keys without issues
 */
TEST_CASE(debug_key_loading) {
    DEBUG_PRINT("Testing key loading from fixtures");
    
    // Initialize library first
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, init_result.error);
    
    DEBUG_PRINT("Loading private key");
    char* private_key = load_test_private_key_no_passphrase();
    TEST_ASSERT_NOT_NULL(private_key);
    
    DEBUG_PRINT("Private key loaded, length=%zu", strlen(private_key));
    DEBUG_PRINT("First 50 chars: %.50s", private_key);
    
    DEBUG_PRINT("Loading public key");
    char* public_key = load_test_public_key_no_passphrase();
    TEST_ASSERT_NOT_NULL(public_key);
    
    DEBUG_PRINT("Public key loaded, length=%zu", strlen(public_key));
    DEBUG_PRINT("First 50 chars: %.50s", public_key);
    
    // Cleanup
    free(private_key);
    free(public_key);
    openpgp_cleanup();
    
    DEBUG_PRINT("Key loading test complete");
    return 0;
}

/**
 * Test 3: Memory stress test
 * This tests memory allocation patterns without crypto operations
 */
TEST_CASE(debug_memory_stress) {
    DEBUG_PRINT("Testing memory allocation patterns");
    
    // Test multiple allocations
    void* ptrs[10];
    size_t sizes[] = {100, 1000, 5000, 10000, 50000};
    
    for (int i = 0; i < 5; i++) {
        DEBUG_PRINT("Allocating %zu bytes", sizes[i]);
        ptrs[i] = malloc(sizes[i]);
        TEST_ASSERT_NOT_NULL(ptrs[i]);
        DEBUG_MEM(ptrs[i], sizes[i]);
        
        // Write to memory to test access
        memset(ptrs[i], 0xAA, sizes[i]);
    }
    
    DEBUG_PRINT("Freeing allocations");
    for (int i = 0; i < 5; i++) {
        DEBUG_PRINT("Freeing ptr[%d]=%p", i, ptrs[i]);
        free(ptrs[i]);
        ptrs[i] = NULL;
    }
    
    DEBUG_PRINT("Memory stress test complete");
    return 0;
}

/**
 * Test 4: Bridge communication without crypto operations
 * This tests that basic library bridge calls work
 */
TEST_CASE(debug_bridge_basic) {
    DEBUG_PRINT("Testing basic bridge communication");
    
    // Initialize library first
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, init_result.error);
    
    DEBUG_PRINT("Bridge initialization successful");
    
    // Test that we can perform multiple init/cleanup cycles
    openpgp_cleanup();
    DEBUG_PRINT("First cleanup successful");
    
    init_result = openpgp_init();
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, init_result.error);
    DEBUG_PRINT("Re-initialization successful");
    
    openpgp_cleanup();
    DEBUG_PRINT("Bridge basic test complete");
    return 0;
}

/**
 * Test 5: Minimal sign operation with known key
 * Test signing with hardcoded fixture key - simplest crypto operation
 */
TEST_CASE(debug_minimal_sign) {
    DEBUG_PRINT("Testing minimal sign operation");
    
    // Initialize library
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, init_result.error);
    
    DEBUG_PRINT("Loading fixture private key");
    char* private_key = load_test_private_key_no_passphrase();
    TEST_ASSERT_NOT_NULL(private_key);
    
    DEBUG_PRINT("Testing signing empty string");
    openpgp_result_t result = openpgp_sign("", private_key, NULL, NULL);
    DEBUG_PRINT("Empty string sign result: error=%d", result.error);
    
    if (result.error == OPENPGP_SUCCESS) {
        DEBUG_PRINT("✓ Empty string signing succeeded");
        if (result.data) {
            DEBUG_PRINT("✓ Got signature data (%zu chars)", strlen((char*)result.data));
            openpgp_result_free(&result);
        }
    } else {
        DEBUG_PRINT("Empty string signing failed: %s", 
                   result.error_message ? result.error_message : "Unknown");
        openpgp_result_free(&result);
    }
    
    DEBUG_PRINT("Testing signing single character");
    result = openpgp_sign("a", private_key, NULL, NULL);
    DEBUG_PRINT("Single char sign result: error=%d", result.error);
    
    if (result.error == OPENPGP_SUCCESS) {
        DEBUG_PRINT("✓ Single character signing succeeded");
        openpgp_result_free(&result);
    } else {
        DEBUG_PRINT("Single character signing failed: %s", 
                   result.error_message ? result.error_message : "Unknown");
        openpgp_result_free(&result);
    }
    
    DEBUG_PRINT("Testing signing 'test' message");
    result = openpgp_sign("test", private_key, NULL, NULL);
    DEBUG_PRINT("'test' sign result: error=%d", result.error);
    
    if (result.error == OPENPGP_SUCCESS) {
        DEBUG_PRINT("✓ 'test' message signing succeeded");
        openpgp_result_free(&result);
    } else {
        DEBUG_PRINT("'test' message signing failed: %s", 
                   result.error_message ? result.error_message : "Unknown");
        openpgp_result_free(&result);
    }
    
    // Cleanup
    free(private_key);
    openpgp_cleanup();
    
    DEBUG_PRINT("Minimal sign test complete");
    return 0;
}

/**
 * Test 6: Test signing with generated keys (segfault reproduction)
 * This test attempts to reproduce the segfault by generating keys and signing
 */
TEST_CASE(debug_generated_key_signing) {
    DEBUG_PRINT("Testing signing with generated keys");
    
    // Initialize library
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, init_result.error);
    
    DEBUG_PRINT("Generating minimal RSA keypair");
    
    // Generate a simple RSA key without passphrase
    openpgp_options_t options;
    openpgp_options_init_default(&options);
    options.name = "Debug Test User";
    options.email = "debug@test.com";
    options.passphrase = NULL; // No passphrase
    options.comment = "Debug Test Key";
    options.key_options.algorithm = OPENPGP_ALGORITHM_RSA;
    options.key_options.rsa_bits = 2048;
    options.key_options.hash = OPENPGP_HASH_SHA256;
    
    DEBUG_PRINT("Calling openpgp_generate_key_with_options()");
    openpgp_result_t gen_result = openpgp_generate_key_with_options(&options);
    
    if (gen_result.error != OPENPGP_SUCCESS) {
        DEBUG_PRINT("Key generation failed: %s", 
                   gen_result.error_message ? gen_result.error_message : "Unknown error");
        openpgp_result_free(&gen_result);
        openpgp_cleanup();
        return 0; // Skip test if key generation fails
    }
    
    DEBUG_PRINT("Key generation successful");
    TEST_ASSERT_NOT_NULL(gen_result.data);
    
    openpgp_keypair_t* keypair = (openpgp_keypair_t*)gen_result.data;
    TEST_ASSERT_NOT_NULL(keypair->private_key);
    
    DEBUG_PRINT("Generated private key length: %zu", strlen(keypair->private_key));
    DEBUG_PRINT("First 50 chars: %.50s", keypair->private_key);
    
    // Now try signing with the generated key (this is where segfault likely occurs)
    DEBUG_PRINT("Attempting to sign with generated key...");
    
    openpgp_result_t sign_result = openpgp_sign("test", keypair->private_key, NULL, NULL);
    
    DEBUG_PRINT("Sign result: error=%d", sign_result.error);
    
    if (sign_result.error == OPENPGP_SUCCESS) {
        DEBUG_PRINT("✓ Generated key signing succeeded!");
        if (sign_result.data) {
            DEBUG_PRINT("✓ Got signature data (%zu chars)", strlen((char*)sign_result.data));
        }
    } else {
        DEBUG_PRINT("✗ Generated key signing failed: %s", 
                   sign_result.error_message ? sign_result.error_message : "Unknown error");
    }
    
    openpgp_result_free(&sign_result);
    openpgp_result_free(&gen_result);
    openpgp_cleanup();
    
    DEBUG_PRINT("Generated key signing test complete");
    return 0;
}

/**
 * Debug test runner function - called from main test runner
 */
void run_debug_tests(void) {
    printf(COLOR_BLUE "\n=== Debug Test Suite ===" COLOR_RESET "\n");
    
    printf(COLOR_BLUE "Debug Configuration:" COLOR_RESET "\n");
    printf("- Verbose debug: %s\n", DEBUG_VERBOSE ? "ON" : "OFF");
    printf("- Memory debug: %s\n", DEBUG_MEMORY ? "ON" : "OFF");
    printf("\n");
    
    // Run debug tests
    RUN_TEST(debug_library_init);
    RUN_TEST(debug_key_loading);
    RUN_TEST(debug_memory_stress);
    RUN_TEST(debug_bridge_basic);
    RUN_TEST(debug_minimal_sign);
    RUN_TEST(debug_generated_key_signing);
    
    printf("=== Debug Tests Complete ===" COLOR_RESET "\n\n");
}