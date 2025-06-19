#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c/include/openpgp.h"

// Declare the key generation integration test functions
extern int test_rsa_2048_generation_and_usage(void);
extern int test_rsa_2048_with_passphrase_generation_and_usage(void);
extern int test_ecdsa_p256_generation_and_usage(void);
extern int test_ed25519_generation_and_usage(void);
extern int test_multi_recipient_with_generated_keys(void);
extern int test_key_metadata_extraction_on_generated_keys(void);

int main() {
    printf("=== Running Key Generation Integration Tests ===\n");
    
    // Run all the key generation integration tests that happen before signing
    printf("Running test_rsa_2048_generation_and_usage...\n");
    test_rsa_2048_generation_and_usage();
    
    printf("Running test_rsa_2048_with_passphrase_generation_and_usage...\n");
    test_rsa_2048_with_passphrase_generation_and_usage();
    
    printf("Running test_ecdsa_p256_generation_and_usage...\n");  
    test_ecdsa_p256_generation_and_usage();
    
    printf("Running test_ed25519_generation_and_usage...\n");
    test_ed25519_generation_and_usage();
    
    printf("Running test_multi_recipient_with_generated_keys...\n");
    test_multi_recipient_with_generated_keys();
    
    printf("Running test_key_metadata_extraction_on_generated_keys...\n");
    test_key_metadata_extraction_on_generated_keys();
    
    printf("\n=== Key Generation Tests Complete ===\n");
    printf("Now testing sign_data after key generation tests...\n");
    
    // Now try the failing operation
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != 0) {
        printf("Init failed after key generation tests: %d\n", init_result.error);
        return 1;
    }
    openpgp_result_free(&init_result);
    
    openpgp_result_t gen_result = openpgp_generate_key("Test User", "test@example.com", NULL);
    if (gen_result.error != 0) {
        printf("Generate failed after key generation tests: %d\n", gen_result.error);
        openpgp_cleanup();
        return 1;
    }
    
    openpgp_keypair_t* keypair = (openpgp_keypair_t*)gen_result.data;
    printf("Key generated successfully, testing sign_data...\n");
    
    openpgp_result_t sign_result = openpgp_sign_data("test", keypair->private_key, NULL, NULL);
    printf("Sign result: %d\n", sign_result.error);
    
    if (sign_result.error == 0) {
        printf("SUCCESS: sign_data works after key generation tests\n");
    } else {
        printf("FAILURE: sign_data failed after key generation tests\n");
    }
    
    openpgp_result_free(&gen_result);
    openpgp_result_free(&sign_result);
    openpgp_cleanup();
    
    return 0;
}