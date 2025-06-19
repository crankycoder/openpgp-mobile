#include "openpgp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("Testing asymmetric encryption with complete GPG-generated no-passphrase keys...\n");

    // Initialize the library
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("Failed to initialize: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 1;
    }
    printf("Library initialized successfully\n");
    openpgp_result_free(&init_result);

    const char *test_message = "hello world";
    
    // Complete GPG-generated public key (no passphrase)
    const char *test_public_key = 
        "-----BEGIN PGP PUBLIC KEY BLOCK-----\n"
        "\n"
        "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\n"
        "EBg=\n"
        "=cq+Z\n"
        "-----END PGP PUBLIC KEY BLOCK-----";

    // Encrypt with the public key  
    printf("Attempting to encrypt message: \"%s\"\n", test_message);
    const char *recipient_keys[] = {test_public_key};
    openpgp_result_t encrypt_result = openpgp_encrypt(test_message, recipient_keys, 1, NULL);
    
    printf("Encryption result:\n");
    printf("  Error code: %d\n", encrypt_result.error);
    printf("  Error message: %s\n", encrypt_result.error_message ? encrypt_result.error_message : "NULL");
    
    if (encrypt_result.error == OPENPGP_SUCCESS) {
        printf("  Success! Encrypted message length: %zu\n", encrypt_result.data_size);
        char *encrypted = (char *)encrypt_result.data;
        printf("  Encrypted message (first 100 chars): %.100s\n", encrypted);
        printf("  ✓ GPG-generated public key encryption WORKS!\n");
    } else {
        printf("  Public key encryption failed\n");
        printf("  This may indicate key format issues with the GPG-generated key\n");
    }
    
    openpgp_result_free(&encrypt_result);
    
    // Cleanup
    openpgp_cleanup();
    printf("Test completed\n");
    
    return 0;
}