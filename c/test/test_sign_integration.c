#include "openpgp.h"
#include "test_framework.h"
#include <string.h>
#include <stdio.h>

static const char *test_message = "Hello, this is a test message for signing!";

// Helper function to validate PGP signature format
static bool validate_pgp_signature(const char *signature) {
    if (!signature) return false;
    
    // Check for cleartext signature format
    if (strstr(signature, "-----BEGIN PGP SIGNED MESSAGE-----") &&
        strstr(signature, "-----BEGIN PGP SIGNATURE-----") &&
        strstr(signature, "-----END PGP SIGNATURE-----")) {
        return true;
    }
    
    // Check for detached signature format
    if (strstr(signature, "-----BEGIN PGP SIGNATURE-----") &&
        strstr(signature, "-----END PGP SIGNATURE-----")) {
        return true;
    }
    
    return false;
}

TEST_CASE(sign_with_generated_rsa_key) {
    printf("Running test: Generate RSA key and sign message...\n");
    
    // Initialize library
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("Failed to initialize library: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return -1;
    }
    openpgp_result_free(&init_result);
    
    // Generate RSA key pair
    openpgp_options_t options;
    openpgp_options_init_default(&options);
    options.name = "Test Signer";
    options.email = "test@example.com";
    options.passphrase = NULL; // No passphrase for simplicity
    options.key_options.algorithm = OPENPGP_ALGORITHM_RSA;
    options.key_options.rsa_bits = 2048;
    
    printf("  Generating RSA 2048-bit keypair...\n");
    openpgp_result_t gen_result = openpgp_generate_key_with_options(&options);
    
    if (gen_result.error != OPENPGP_SUCCESS) {
        printf("  ✗ Key generation failed: %s\n", 
               gen_result.error_message ? gen_result.error_message : "Unknown error");
        openpgp_result_free(&gen_result);
        openpgp_cleanup();
        return -1;
    }
    
    openpgp_keypair_t* keypair = (openpgp_keypair_t*)gen_result.data;
    if (!keypair || !keypair->private_key || !keypair->public_key) {
        printf("  ✗ Invalid keypair generated\n");
        openpgp_result_free(&gen_result);
        openpgp_cleanup();
        return -1;
    }
    
    printf("  ✓ RSA keypair generated successfully\n");
    
    // Now test signing with the generated key
    printf("  Testing message signing...\n");
    openpgp_result_t sign_result = openpgp_sign(
        test_message,
        keypair->private_key,
        NULL,  // No passphrase
        NULL   // Default options
    );
    
    bool test_passed = false;
    if (sign_result.error == OPENPGP_SUCCESS && sign_result.data) {
        char* signature = (char*)sign_result.data;
        if (validate_pgp_signature(signature)) {
            printf("  ✓ Generated valid PGP signature\n");
            printf("  ✓ Signature format validated\n");
            test_passed = true;
        } else {
            printf("  ✗ Invalid signature format\n");
            printf("  Signature preview: %.100s...\n", signature);
        }
    } else {
        printf("  ✗ Signing failed: %s\n", 
               sign_result.error_message ? sign_result.error_message : "Unknown error");
    }
    
    // Cleanup
    openpgp_result_free(&gen_result);
    openpgp_result_free(&sign_result);
    openpgp_cleanup();
    
    return test_passed ? 0 : -1;
}

TEST_CASE(sign_data_with_generated_key) {
    printf("Running test: Generate key and test sign_data...\n");
    
    // Initialize library
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("Failed to initialize library: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return -1;
    }
    openpgp_result_free(&init_result);
    
    // Generate simple key
    openpgp_result_t gen_result = openpgp_generate_key("Test User", "test@example.com", NULL);
    
    if (gen_result.error != OPENPGP_SUCCESS) {
        printf("  ✗ Key generation failed: %s\n", 
               gen_result.error_message ? gen_result.error_message : "Unknown error");
        openpgp_result_free(&gen_result);
        openpgp_cleanup();
        return -1;
    }
    
    openpgp_keypair_t* keypair = (openpgp_keypair_t*)gen_result.data;
    printf("  ✓ Keypair generated successfully\n");
    
    // Test sign_data function
    printf("  Testing sign_data function...\n");
    openpgp_result_t sign_result = openpgp_sign_data(
        test_message,
        keypair->private_key,
        NULL,  // No passphrase
        NULL   // Default options
    );
    
    bool test_passed = false;
    if (sign_result.error == OPENPGP_SUCCESS && sign_result.data) {
        char* signature = (char*)sign_result.data;
        if (validate_pgp_signature(signature)) {
            printf("  ✓ sign_data generated valid signature\n");
            test_passed = true;
        } else {
            printf("  ✗ sign_data produced invalid signature format\n");
        }
    } else {
        printf("  ✗ sign_data failed: %s\n", 
               sign_result.error_message ? sign_result.error_message : "Unknown error");
    }
    
    // Cleanup
    openpgp_result_free(&gen_result);
    openpgp_result_free(&sign_result);
    openpgp_cleanup();
    
    return test_passed ? 0 : -1;
}

// Test runner for integration signing tests
void run_sign_integration_tests(void) {
    printf("\n=== Sign Integration Tests ===\n");
    
    RUN_TEST(sign_with_generated_rsa_key);
    RUN_TEST(sign_data_with_generated_key);
    
    printf("=== Sign Integration Tests Complete ===\n\n");
}