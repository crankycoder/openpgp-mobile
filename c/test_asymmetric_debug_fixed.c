#include "openpgp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("Testing asymmetric encryption with correct Go test keys...\n");

    // Initialize the library
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("Failed to initialize: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 1;
    }
    printf("Library initialized successfully\n");
    openpgp_result_free(&init_result);

    const char *test_message = "Hello, asymmetric encryption!";
    
    // Use the EXACT key from Go decrypt_test.go
    const char *test_private_key = 
        "-----BEGIN PGP PRIVATE KEY BLOCK-----\n"
        "\n"
        "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\n"
        "=oWIw\n"
        "-----END PGP PRIVATE KEY BLOCK-----";

    // Get the corresponding public key from the private key
    printf("Extracting public key from private key...\n");
    openpgp_result_t public_key_result = openpgp_convert_private_to_public(test_private_key);
    if (public_key_result.error != OPENPGP_SUCCESS) {
        printf("Failed to convert private to public key: %s\n", public_key_result.error_message);
        openpgp_result_free(&public_key_result);
        openpgp_cleanup();
        return 1;
    }
    char *public_key = (char *)public_key_result.data;
    printf("Successfully extracted public key\n");

    // Encrypt with the public key  
    printf("Attempting to encrypt message: \"%s\"\n", test_message);
    const char *recipient_keys[] = {public_key};
    openpgp_result_t encrypt_result = openpgp_encrypt(test_message, recipient_keys, 1, NULL);
    
    printf("Encryption result:\n");
    printf("  Error code: %d\n", encrypt_result.error);
    printf("  Error message: %s\n", encrypt_result.error_message ? encrypt_result.error_message : "NULL");
    
    if (encrypt_result.error == OPENPGP_SUCCESS) {
        printf("  Success! Encrypted message length: %zu\n", encrypt_result.data_size);
        char *encrypted = (char *)encrypt_result.data;
        printf("  Encrypted message (first 100 chars): %.100s\n", encrypted);
        
        printf("\nAttempting to decrypt the encrypted message with passphrase 'test'...\n");
        
        // Try decryption with the correct passphrase from Go tests
        openpgp_result_t decrypt_result = openpgp_decrypt(encrypted, test_private_key, "test", NULL);
        
        printf("Decryption result:\n");
        printf("  Error code: %d\n", decrypt_result.error);
        printf("  Error message: %s\n", decrypt_result.error_message ? decrypt_result.error_message : "NULL");
        
        if (decrypt_result.error == OPENPGP_SUCCESS) {
            printf("  Success! Decrypted message: \"%s\"\n", (char *)decrypt_result.data);
            
            // Verify round-trip
            if (strcmp(test_message, (char *)decrypt_result.data) == 0) {
                printf("  ✓ Round-trip encryption/decryption successful!\n");
            } else {
                printf("  ✗ Round-trip failed - messages don't match!\n");
                printf("    Original: %s\n", test_message);
                printf("    Decrypted: %s\n", (char *)decrypt_result.data);
            }
        } else {
            printf("  Decryption failed\n");
        }
        
        openpgp_result_free(&decrypt_result);
    } else {
        printf("  Encryption failed\n");
    }
    
    openpgp_result_free(&encrypt_result);
    openpgp_result_free(&public_key_result);
    
    // Cleanup
    openpgp_cleanup();
    printf("Test completed\n");
    
    return 0;
}