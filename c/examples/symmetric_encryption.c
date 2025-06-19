#include "openpgp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * Symmetric Encryption Example
 * 
 * This example demonstrates how to use the OpenPGP C wrapper library
 * for symmetric encryption and decryption operations.
 */

int main(void) {
    printf("OpenPGP Symmetric Encryption Example\n");
    printf("====================================\n\n");

    /* Initialize the library */
    printf("1. Initializing OpenPGP library...\n");
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("   Failed to initialize: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 1;
    }
    printf("   Library initialized successfully!\n\n");
    openpgp_result_free(&init_result);

    /* Example data */
    const char *message = "Hello, world! This is a secret message.";
    const char *passphrase = "my_secure_passphrase";

    printf("2. Encrypting message with symmetric encryption...\n");
    printf("   Message: \"%s\"\n", message);
    printf("   Passphrase: \"%s\"\n", passphrase);

    /* Encrypt the message */
    openpgp_result_t encrypt_result = openpgp_encrypt_symmetric(
        message, passphrase, NULL, NULL);
    
    if (encrypt_result.error != OPENPGP_SUCCESS) {
        printf("   Encryption failed: %s\n", encrypt_result.error_message);
        openpgp_result_free(&encrypt_result);
        openpgp_cleanup();
        return 1;
    }

    char *encrypted_message = (char *)encrypt_result.data;
    printf("   Encryption successful!\n");
    printf("   Encrypted message length: %zu bytes\n", encrypt_result.data_size);
    printf("   Encrypted message (first 100 chars): %.100s...\n\n", encrypted_message);

    /* Decrypt the message */
    printf("3. Decrypting message...\n");
    openpgp_result_t decrypt_result = openpgp_decrypt_symmetric(
        encrypted_message, passphrase, NULL);
    
    if (decrypt_result.error != OPENPGP_SUCCESS) {
        printf("   Decryption failed: %s\n", decrypt_result.error_message);
        openpgp_result_free(&decrypt_result);
        openpgp_result_free(&encrypt_result);
        openpgp_cleanup();
        return 1;
    }

    char *decrypted_message = (char *)decrypt_result.data;
    printf("   Decryption successful!\n");
    printf("   Decrypted message: \"%s\"\n", decrypted_message);

    /* Verify the messages match */
    if (strcmp(message, decrypted_message) == 0) {
        printf("   ✓ Original and decrypted messages match!\n\n");
    } else {
        printf("   ✗ Messages do not match!\n\n");
    }

    /* Example with custom options */
    printf("4. Encrypting with custom options...\n");
    
    openpgp_key_options_t options;
    openpgp_key_options_init_default(&options);
    options.cipher = OPENPGP_CIPHER_AES256;
    options.hash = OPENPGP_HASH_SHA256;
    options.compression = OPENPGP_COMPRESSION_ZLIB;
    options.compression_level = 6;

    openpgp_file_hints_t hints = {
        .is_binary = false,
        .file_name = "secret.txt",
        .mod_time = "2023-01-01T00:00:00Z"
    };

    openpgp_result_t encrypt_options_result = openpgp_encrypt_symmetric(
        message, passphrase, &hints, &options);
    
    if (encrypt_options_result.error == OPENPGP_SUCCESS) {
        printf("   Encryption with options successful!\n");
        printf("   Used AES256 cipher with SHA256 hash and ZLIB compression\n");
        
        openpgp_result_t decrypt_options_result = openpgp_decrypt_symmetric(
            (char *)encrypt_options_result.data, passphrase, &options);
        
        if (decrypt_options_result.error == OPENPGP_SUCCESS) {
            printf("   Decryption with options successful!\n");
            printf("   Message: \"%s\"\n", (char *)decrypt_options_result.data);
        } else {
            printf("   Decryption with options failed: %s\n", decrypt_options_result.error_message);
        }
        
        openpgp_result_free(&decrypt_options_result);
    } else {
        printf("   Encryption with options failed: %s\n", encrypt_options_result.error_message);
    }

    printf("\n5. Example with binary data...\n");
    
    /* Example binary data */
    uint8_t binary_data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    size_t binary_len = sizeof(binary_data);
    
    printf("   Binary data: ");
    for (size_t i = 0; i < binary_len; i++) {
        printf("%02X ", binary_data[i]);
    }
    printf("\n");

    openpgp_result_t encrypt_bytes_result = openpgp_encrypt_symmetric_bytes(
        binary_data, binary_len, passphrase, NULL, NULL);
    
    if (encrypt_bytes_result.error == OPENPGP_SUCCESS) {
        printf("   Binary encryption successful!\n");
        
        openpgp_result_t decrypt_bytes_result = openpgp_decrypt_symmetric_bytes(
            (uint8_t *)encrypt_bytes_result.data, encrypt_bytes_result.data_size, 
            passphrase, NULL);
        
        if (decrypt_bytes_result.error == OPENPGP_SUCCESS) {
            printf("   Binary decryption successful!\n");
            printf("   Decrypted data: ");
            uint8_t *decrypted_data = (uint8_t *)decrypt_bytes_result.data;
            for (size_t i = 0; i < decrypt_bytes_result.data_size; i++) {
                printf("%02X ", decrypted_data[i]);
            }
            printf("\n");
            
            /* Verify binary data matches */
            if (decrypt_bytes_result.data_size == binary_len && 
                memcmp(binary_data, decrypted_data, binary_len) == 0) {
                printf("   ✓ Original and decrypted binary data match!\n");
            } else {
                printf("   ✗ Binary data does not match!\n");
            }
        } else {
            printf("   Binary decryption failed: %s\n", decrypt_bytes_result.error_message);
        }
        
        openpgp_result_free(&decrypt_bytes_result);
    } else {
        printf("   Binary encryption failed: %s\n", encrypt_bytes_result.error_message);
    }

    /* Clean up */
    openpgp_result_free(&encrypt_result);
    openpgp_result_free(&decrypt_result);
    openpgp_result_free(&encrypt_options_result);
    openpgp_result_free(&encrypt_bytes_result);

    printf("\n6. Cleaning up...\n");
    openpgp_cleanup();
    printf("   Library cleanup complete!\n");

    printf("\nExample completed successfully!\n");
    printf("\nNote: This example will show bridge connection errors\n");
    printf("until the Go bridge library is available.\n");
    
    return 0;
}