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

/* Global test counters - required by test framework */
int g_tests_run = 0;        /* Total assertions/subtests */
int g_tests_failed = 0;     /* Failed assertions/subtests */
int g_major_tests_run = 0;  /* Major test functions */
int g_major_tests_failed = 0; /* Failed major test functions */

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
 * Test runner for debug tests
 */
int main(void) {
    printf(COLOR_BLUE "OpenPGP Debug Test Suite" COLOR_RESET "\n");
    printf("================================\n\n");
    
    printf(COLOR_BLUE "Debug Configuration:" COLOR_RESET "\n");
    printf("- Verbose debug: %s\n", DEBUG_VERBOSE ? "ON" : "OFF");
    printf("- Memory debug: %s\n", DEBUG_MEMORY ? "ON" : "OFF");
    printf("- Working directory: %s\n", getcwd(NULL, 0));
    printf("\n");
    
    // Run debug tests
    RUN_TEST(debug_library_init);
    RUN_TEST(debug_key_loading);
    RUN_TEST(debug_memory_stress);
    RUN_TEST(debug_bridge_basic);
    RUN_TEST(debug_minimal_sign);
    
    printf("\n" COLOR_BLUE "Debug Test Results:" COLOR_RESET "\n");
    printf("Major tests: %d run, %d failed\n", g_major_tests_run, g_major_tests_failed);
    printf("Total assertions: %d run, %d failed\n", g_tests_run, g_tests_failed);
    
    if (g_major_tests_failed == 0) {
        printf(COLOR_GREEN "All debug tests passed!" COLOR_RESET "\n");
        return 0;
    } else {
        printf(COLOR_RED "Some debug tests failed!" COLOR_RESET "\n");
        return 1;
    }
}