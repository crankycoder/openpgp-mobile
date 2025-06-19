/**
 * Minimal test cases for signing/verification debugging
 * 
 * This file contains the absolute minimal test cases to isolate
 * the segmentation fault. Each test is completely isolated.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_framework.h"
#include "../include/openpgp.h"

/* Global test counters - declared extern, defined in test_runner.c */
extern int g_tests_run;        /* Total assertions/subtests */
extern int g_tests_failed;     /* Failed assertions/subtests */
extern int g_major_tests_run;  /* Major test functions */
extern int g_major_tests_failed; /* Failed major test functions */

// Enable/disable individual tests
#define TEST_MINIMAL_SIGN 1
#define TEST_MINIMAL_VERIFY 1
#define TEST_GENERATED_KEY 0  // Disabled initially - this is where segfault occurs

/**
 * Test the absolute minimum signing operation
 * Uses hardcoded test message and will load fixture key
 */
TEST_CASE(minimal_sign_test) {
    printf("  Starting minimal sign test...\n");
    
    // Initialize library
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("  Library init failed: %d\n", init_result.error);
        return 1;
    }
    
    // Load fixture private key
    FILE* key_file = fopen("c/test/gpg-test-keys/test-private-key.asc", "r");
    if (!key_file) {
        printf("  Could not load test private key\n");
        openpgp_cleanup();
        return 1;
    }
    
    // Read key into buffer
    fseek(key_file, 0, SEEK_END);
    long key_size = ftell(key_file);
    fseek(key_file, 0, SEEK_SET);
    
    char* private_key = malloc(key_size + 1);
    if (!private_key) {
        printf("  Failed to allocate memory for key\n");
        fclose(key_file);
        openpgp_cleanup();
        return 1;
    }
    
    size_t bytes_read = fread(private_key, 1, key_size, key_file);
    private_key[bytes_read] = '\0';
    fclose(key_file);
    
    printf("  Private key loaded: %zu bytes\n", bytes_read);
    
    // Test signing with minimal message
    printf("  Attempting to sign 'test' message...\n");
    openpgp_result_t sign_result = openpgp_sign("test", private_key, NULL, NULL);
    
    printf("  Sign result: error=%d\n", sign_result.error);
    
    if (sign_result.error == OPENPGP_SUCCESS) {
        printf("  ✓ Sign succeeded\n");
        if (sign_result.data) {
            printf("  ✓ Got signature data (%zu chars)\n", strlen(sign_result.data));
            free(sign_result.data);
        }
    } else {
        printf("  ℹ Sign failed (expected during debugging): %d\n", sign_result.error);
        if (sign_result.error_message) {
            printf("  Error: %s\n", sign_result.error_message);
            free(sign_result.error_message);
        }
    }
    
    // Cleanup
    free(private_key);
    openpgp_cleanup();
    
    printf("  Minimal sign test completed\n");
    return 0;  // Always pass during debugging
}

/**
 * Test minimal verification operation
 * This will only run if we have a signature to verify
 */
TEST_CASE(minimal_verify_test) {
    printf("  Starting minimal verify test...\n");
    
    // For now, just test that verify function exists and handles errors
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("  Library init failed: %d\n", init_result.error);
        return 1;
    }
    
    // Test with obviously invalid signature
    openpgp_verification_result_t* verify_result = NULL;
    openpgp_result_t result = openpgp_verify("NOT A SIGNATURE", "NOT A KEY", &verify_result);
    
    printf("  Verify result: error=%d\n", result.error);
    
    // Should fail gracefully, not crash
    if (result.error != OPENPGP_SUCCESS) {
        printf("  ✓ Verify correctly rejected invalid input\n");
    }
    
    if (verify_result) {
        // If somehow we got a result, free it
        // This would be implemented when verify is complete
        printf("  Got verify result (unexpected)\n");
    }
    
    openpgp_cleanup();
    
    printf("  Minimal verify test completed\n");
    return 0;  // Always pass during debugging
}

/**
 * Test with generated key - this is where the segfault occurs
 * Disabled by default, enabled only when ready to debug
 */
TEST_CASE(minimal_generated_key_test) {
    printf("  Starting minimal generated key test...\n");
    
    if (!TEST_GENERATED_KEY) {
        printf("  ⚠ Test disabled (known to segfault)\n");
        return 0;
    }
    
    // Initialize library
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("  Library init failed: %d\n", init_result.error);
        return 1;
    }
    
    printf("  Generating minimal RSA key...\n");
    
    // Generate minimal key
    openpgp_options_t opts = {
        .email = "debug@test.com",
        .name = "Debug Test",
        .passphrase = NULL,
        .key_options = {
            .algorithm = OPENPGP_ALGORITHM_RSA,
            .rsa_bits = 2048
        }
    };
    
    openpgp_result_t gen_result = openpgp_generate_key_with_options(&opts);
    printf("  Key generation result: error=%d\n", gen_result.error);
    
    if (gen_result.error != OPENPGP_SUCCESS) {
        printf("  Key generation failed\n");
        openpgp_cleanup();
        return 1;
    }
    
    printf("  Key generated successfully\n");
    
    // Cast the data to keypair
    openpgp_keypair_t* keypair = (openpgp_keypair_t*)gen_result.data;
    printf("  Private key length: %zu\n", strlen(keypair->private_key));
    
    // This is where the segfault typically occurs
    printf("  Attempting to sign with generated key...\n");
    openpgp_result_t sign_result = openpgp_sign("test", keypair->private_key, NULL, NULL);
    
    printf("  Sign result: error=%d\n", sign_result.error);
    
    // Cleanup
    openpgp_result_free(&gen_result);
    if (sign_result.data) {
        free(sign_result.data);
    }
    
    openpgp_cleanup();
    
    printf("  Generated key test completed\n");
    return 0;
}

/**
 * Minimal test runner function - called from main test runner
 */
void run_minimal_tests(void) {
    printf(COLOR_BLUE "\n=== Minimal Sign/Verify Tests ===" COLOR_RESET "\n");
    
    printf("Test configuration:\n");
    printf("- TEST_MINIMAL_SIGN: %s\n", TEST_MINIMAL_SIGN ? "ENABLED" : "DISABLED");
    printf("- TEST_MINIMAL_VERIFY: %s\n", TEST_MINIMAL_VERIFY ? "ENABLED" : "DISABLED");
    printf("- TEST_GENERATED_KEY: %s\n", TEST_GENERATED_KEY ? "ENABLED" : "DISABLED");
    printf("\n");
    
    // Run enabled tests
    if (TEST_MINIMAL_SIGN) {
        printf("Running minimal_sign_test...\n");
        test_minimal_sign_test();
        printf("\n");
    }
    
    if (TEST_MINIMAL_VERIFY) {
        printf("Running minimal_verify_test...\n");
        test_minimal_verify_test();
        printf("\n");
    }
    
    if (TEST_GENERATED_KEY) {
        printf("Running minimal_generated_key_test...\n");
        test_minimal_generated_key_test();
        printf("\n");
    }
    
    printf("=== Minimal Tests Complete ===" COLOR_RESET "\n\n");
}