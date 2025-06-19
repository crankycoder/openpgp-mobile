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
    
    // Check for signed message format (used by sign_data)
    if (strstr(signature, "-----BEGIN PGP MESSAGE-----") &&
        strstr(signature, "-----END PGP MESSAGE-----")) {
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
    
    printf("  DEBUG: gen_result.data = %p\n", gen_result.data);
    fflush(stdout);
    printf("  ✓ Keypair");
    fflush(stdout);
    if (!gen_result.data) {
        printf("  ✗ gen_result.data is NULL!\n");
        openpgp_result_free(&gen_result);
        openpgp_cleanup();
        return -1;
    }
    
    printf("  DEBUG: gen_result.data points to valid memory, attempting cast...\n");
    fflush(stdout);
    
    // Add memory validation before casting
    printf("  DEBUG: Checking memory at gen_result.data = %p\n", gen_result.data);
    fflush(stdout);
    
    // Try to access the first byte to see if memory is readable
    volatile unsigned char* test_ptr = (volatile unsigned char*)gen_result.data;
    printf("  DEBUG: First byte at address: 0x%02x\n", *test_ptr);
    fflush(stdout);
    
    openpgp_keypair_t* keypair = (openpgp_keypair_t*)gen_result.data;
    printf(" generated successfully\n");
    printf("  DEBUG: Cast successful, checking keypair structure...\n");
    fflush(stdout);
    
    printf("  DEBUG: keypair pointer = %p\n", (void*)keypair);
    fflush(stdout);
    
    // Safety checks
    if (!keypair) {
        printf("  ✗ Keypair is NULL\n");
        openpgp_result_free(&gen_result);
        openpgp_cleanup();
        return -1;
    }
    
    printf("  DEBUG: keypair is not NULL, checking private_key field...\n");
    fflush(stdout);
    
    // Try to access the private_key field carefully
    printf("  DEBUG: About to read keypair->private_key address...\n");
    fflush(stdout);
    
    char* private_key_ptr = keypair->private_key;
    printf("  DEBUG: keypair->private_key = %p\n", (void*)private_key_ptr);
    fflush(stdout);
    
    if (!private_key_ptr) {
        printf("  ✗ Private key is NULL\n");
        openpgp_result_free(&gen_result);
        openpgp_cleanup();
        return -1;
    }
    
    printf("  DEBUG: private_key is not NULL, checking if readable...\n");
    fflush(stdout);
    
    // Try to read the first character of the private key string
    printf("  DEBUG: Attempting to read first char of private key...\n");
    fflush(stdout);
    
    char first_char = private_key_ptr[0];
    printf("  DEBUG: First character of private key: '%c' (0x%02x)\n", 
           (first_char >= 32 && first_char <= 126) ? first_char : '?', 
           (unsigned char)first_char);
    fflush(stdout);
    
    printf("  DEBUG: About to call strlen on private key...\n");
    fflush(stdout);
    
    size_t key_len = strlen(private_key_ptr);
    printf("  DEBUG: Private key length: %zu characters\n", key_len);
    fflush(stdout);
    
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
            printf("  Debug: First 200 chars of signature:\n");
            printf("  %.200s\n", signature);
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

TEST_CASE(sign_bytes_with_generated_key) {
    printf("Running test: Generate key and test sign_bytes...\n");
    
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
    
    // Test sign_bytes function with binary data
    printf("  Testing sign_bytes function...\n");
    const uint8_t *test_data = (const uint8_t*)test_message;
    size_t data_len = strlen(test_message);
    
    openpgp_result_t sign_result = openpgp_sign_bytes(
        test_data,
        data_len,
        keypair->private_key,
        NULL,  // No passphrase
        NULL   // Default options
    );
    
    bool test_passed = false;
    if (sign_result.error == OPENPGP_SUCCESS && sign_result.data) {
        char* signature = (char*)sign_result.data;
        if (validate_pgp_signature(signature)) {
            printf("  ✓ sign_bytes generated valid signature\n");
            test_passed = true;
        } else {
            printf("  ✗ sign_bytes produced invalid signature format\n");
            printf("  Debug: Signature length: %zu\n", strlen(signature));
            // Check if it's ASCII
            bool is_ascii = true;
            for (int i = 0; i < 50 && signature[i]; i++) {
                if (signature[i] < 32 && signature[i] != '\n' && signature[i] != '\r' && signature[i] != '\t') {
                    is_ascii = false;
                    break;
                }
            }
            if (is_ascii) {
                printf("  Debug: First 200 chars of signature:\n");
                printf("  %.200s\n", signature);
            } else {
                printf("  Debug: Signature contains binary data (not ASCII)\n");
            }
        }
    } else {
        printf("  ✗ sign_bytes failed: %s\n", 
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
    RUN_TEST(sign_bytes_with_generated_key);
    
    printf("=== Sign Integration Tests Complete ===\n\n");
}