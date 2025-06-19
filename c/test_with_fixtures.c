#include "openpgp.h"
#include "test/test_fixtures.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("Testing asymmetric encryption with fixture data (no passphrase keys)...\n");

    // Initialize the library
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("Failed to initialize: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 1;
    }
    printf("Library initialized successfully\n");
    openpgp_result_free(&init_result);

    // Load test data from fixture files
    char *test_message = load_test_message();
    char *test_public_key = load_test_public_key_no_passphrase();
    char *test_private_key = load_test_private_key_no_passphrase();
    
    if (!test_message || !test_public_key || !test_private_key) {
        printf("Failed to load fixture data\n");
        free(test_message);
        free(test_public_key);  
        free(test_private_key);
        return 1;
    }
    
    // Remove trailing newline from message if present
    size_t msg_len = strlen(test_message);
    if (msg_len > 0 && test_message[msg_len-1] == '\n') {
        test_message[msg_len-1] = '\0';
    }
    
    printf("Loaded test data:\n");
    printf("  Message: \"%s\"\n", test_message);
    printf("  Public key: %zu bytes\n", strlen(test_public_key));
    printf("  Private key: %zu bytes\n", strlen(test_private_key));

    // Test encryption
    printf("\nAttempting encryption...\n");
    const char *recipient_keys[] = {test_public_key};
    openpgp_result_t encrypt_result = openpgp_encrypt(test_message, recipient_keys, 1, NULL);
    
    printf("Encryption result:\n");
    printf("  Error code: %d\n", encrypt_result.error);
    printf("  Error message: %s\n", encrypt_result.error_message ? encrypt_result.error_message : "NULL");
    
    if (encrypt_result.error == OPENPGP_SUCCESS) {
        printf("  ✓ Success! Encrypted message length: %zu\n", encrypt_result.data_size);
        char *encrypted = (char *)encrypt_result.data;
        printf("  Encrypted message (first 100 chars): %.100s\n", encrypted);
        
        // Test decryption with no passphrase
        printf("\nAttempting decryption (no passphrase)...\n");
        openpgp_result_t decrypt_result = openpgp_decrypt(encrypted, test_private_key, NULL, NULL);
        
        printf("Decryption result:\n");
        printf("  Error code: %d\n", decrypt_result.error);
        printf("  Error message: %s\n", decrypt_result.error_message ? decrypt_result.error_message : "NULL");
        
        if (decrypt_result.error == OPENPGP_SUCCESS) {
            printf("  ✓ Success! Decrypted message: \"%s\"\n", (char *)decrypt_result.data);
            
            // Verify round-trip
            if (strcmp(test_message, (char *)decrypt_result.data) == 0) {
                printf("  ✓ ROUND-TRIP SUCCESS! Messages match perfectly!\n");
            } else {
                printf("  ✗ Round-trip failed - messages don't match\n");
                printf("    Original: '%s'\n", test_message);
                printf("    Decrypted: '%s'\n", (char *)decrypt_result.data);
            }
        } else {
            printf("  ✗ Decryption failed\n");
        }
        
        openpgp_result_free(&decrypt_result);
    } else {
        printf("  ✗ Encryption failed\n");
    }
    
    openpgp_result_free(&encrypt_result);
    
    // Cleanup
    free(test_message);
    free(test_public_key);
    free(test_private_key);
    openpgp_cleanup();
    
    printf("\nTest completed\n");
    return 0;
}