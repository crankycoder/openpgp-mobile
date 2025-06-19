/*
 * key_operations.c - Example usage of key operation functions
 *
 * This file demonstrates how to use the OpenPGP C binding to:
 * - Convert private keys to public keys
 * - Extract metadata from public keys
 * - Extract metadata from private keys
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/openpgp.h"

/* Example RSA private key for demonstration */
static const char* EXAMPLE_PRIVATE_KEY = 
"-----BEGIN PGP PRIVATE KEY BLOCK-----\n"
"\n"
"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\n"
"iERBTiEWBMRjEe0cX6/KjGpkR48=\n"
"=gM4o\n"
"-----END PGP PRIVATE KEY BLOCK-----";void demonstrate_convert_private_to_public() {
    printf("\n=== Converting Private Key to Public Key ===\n");
    
    /* Initialize the library */
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        fprintf(stderr, "Failed to initialize library: %s\n", 
                init_result.error_message ? init_result.error_message : "Unknown error");
        return;
    }
    
    /* Convert private key to public key */
    openpgp_result_t result = openpgp_convert_private_to_public(EXAMPLE_PRIVATE_KEY);
    
    if (result.error == OPENPGP_SUCCESS) {
        char *public_key = (char *)result.data;
        printf("Successfully converted private key to public key!\n");
        printf("Public Key:\n%s\n", public_key);
    } else {
        fprintf(stderr, "Failed to convert key: %s\n", 
                result.error_message ? result.error_message : "Unknown error");
    }
    
    /* Cleanup */
    openpgp_result_free(&result);
    openpgp_cleanup();
}

void demonstrate_public_key_metadata() {
    printf("\n=== Extracting Public Key Metadata ===\n");
    
    /* For this example, we'll first generate a key pair */
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        fprintf(stderr, "Failed to initialize library: %s\n", 
                init_result.error_message ? init_result.error_message : "Unknown error");
        return;
    }
    
    /* Generate a key pair */
    openpgp_result_t gen_result = openpgp_generate_key("Test User", "test@example.com", NULL);
    if (gen_result.error != OPENPGP_SUCCESS) {
        fprintf(stderr, "Failed to generate key: %s\n", 
                gen_result.error_message ? gen_result.error_message : "Unknown error");
        openpgp_cleanup();
        return;
    }
    
    openpgp_keypair_t *keypair = (openpgp_keypair_t *)gen_result.data;
    
    /* Extract metadata from the public key */
    openpgp_result_t meta_result = openpgp_get_public_key_metadata(keypair->public_key);
    
    if (meta_result.error == OPENPGP_SUCCESS) {
        openpgp_public_key_metadata_t *metadata = (openpgp_public_key_metadata_t *)meta_result.data;
        
        printf("Public Key Metadata:\n");
        printf("  Algorithm: %s\n", metadata->algorithm ? metadata->algorithm : "Unknown");
        printf("  Key ID: %s\n", metadata->key_id ? metadata->key_id : "Unknown");
        printf("  Key ID (short): %s\n", metadata->key_id_short ? metadata->key_id_short : "Unknown");
        printf("  Fingerprint: %s\n", metadata->fingerprint ? metadata->fingerprint : "Unknown");
        printf("  Creation Time: %s\n", metadata->creation_time ? metadata->creation_time : "Unknown");
        printf("  Can Sign: %s\n", metadata->can_sign ? "Yes" : "No");
        printf("  Can Encrypt: %s\n", metadata->can_encrypt ? "Yes" : "No");
        printf("  Is Subkey: %s\n", metadata->is_sub_key ? "Yes" : "No");
        
        /* Display identities */
        printf("  Identities: %zu\n", metadata->identities_count);
        for (size_t i = 0; i < metadata->identities_count; i++) {
            openpgp_identity_t *identity = &metadata->identities[i];
            printf("    Identity %zu:\n", i + 1);
            if (identity->name) printf("      Name: %s\n", identity->name);
            if (identity->email) printf("      Email: %s\n", identity->email);
            if (identity->comment) printf("      Comment: %s\n", identity->comment);
        }
        
        /* Free metadata */
        openpgp_public_key_metadata_free(metadata);
    } else {
        fprintf(stderr, "Failed to extract metadata: %s\n", 
                meta_result.error_message ? meta_result.error_message : "Unknown error");
    }
    
    /* Cleanup */
    openpgp_keypair_free(keypair);
    openpgp_result_free(&gen_result);
    openpgp_result_free(&meta_result);
    openpgp_cleanup();
}void demonstrate_private_key_metadata() {
    printf("\n=== Extracting Private Key Metadata ===\n");
    
    /* Initialize the library */
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        fprintf(stderr, "Failed to initialize library: %s\n", 
                init_result.error_message ? init_result.error_message : "Unknown error");
        return;
    }
    
    /* Extract metadata from the example private key */
    openpgp_result_t meta_result = openpgp_get_private_key_metadata(EXAMPLE_PRIVATE_KEY);
    
    if (meta_result.error == OPENPGP_SUCCESS) {
        openpgp_private_key_metadata_t *metadata = (openpgp_private_key_metadata_t *)meta_result.data;
        
        printf("Private Key Metadata:\n");
        printf("  Key ID: %s\n", metadata->key_id ? metadata->key_id : "Unknown");
        printf("  Key ID (short): %s\n", metadata->key_id_short ? metadata->key_id_short : "Unknown");
        printf("  Fingerprint: %s\n", metadata->fingerprint ? metadata->fingerprint : "Unknown");
        printf("  Creation Time: %s\n", metadata->creation_time ? metadata->creation_time : "Unknown");
        printf("  Encrypted: %s\n", metadata->encrypted ? "Yes" : "No");
        printf("  Can Sign: %s\n", metadata->can_sign ? "Yes" : "No");
        printf("  Is Subkey: %s\n", metadata->is_sub_key ? "Yes" : "No");
        
        /* Display identities */
        printf("  Identities: %zu\n", metadata->identities_count);
        for (size_t i = 0; i < metadata->identities_count; i++) {
            openpgp_identity_t *identity = &metadata->identities[i];
            printf("    Identity %zu:\n", i + 1);
            if (identity->name) printf("      Name: %s\n", identity->name);
            if (identity->email) printf("      Email: %s\n", identity->email);
            if (identity->comment) printf("      Comment: %s\n", identity->comment);
        }
        
        /* Free metadata */
        openpgp_private_key_metadata_free(metadata);
    } else {
        fprintf(stderr, "Failed to extract metadata: %s\n", 
                meta_result.error_message ? meta_result.error_message : "Unknown error");
    }
    
    /* Cleanup */
    openpgp_result_free(&meta_result);
    openpgp_cleanup();
}

int main() {
    printf("OpenPGP C Binding - Key Operations Example\n");
    printf("==========================================\n");
    
    /* Demonstrate each operation */
    demonstrate_convert_private_to_public();
    demonstrate_public_key_metadata();
    demonstrate_private_key_metadata();
    
    printf("\nDone!\n");
    return 0;
}