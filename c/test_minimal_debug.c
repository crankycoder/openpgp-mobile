#include "test/test_framework.h"
#include "openpgp.h"
#include <string.h>
#include <stdio.h>

static const char *test_message = "Hello, this is a test message for signing!";

int main() {
    printf("Running minimal debug test...\n");
    
    // Initialize library
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("Failed to initialize library: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return -1;
    }
    openpgp_result_free(&init_result);
    
    // Generate simple key
    printf("Generating key...\n");
    openpgp_result_t gen_result = openpgp_generate_key("Test User", "test@example.com", NULL);
    
    if (gen_result.error != OPENPGP_SUCCESS) {
        printf("Key generation failed: %s\n", 
               gen_result.error_message ? gen_result.error_message : "Unknown error");
        openpgp_result_free(&gen_result);
        openpgp_cleanup();
        return -1;
    }
    
    printf("Key generated successfully\n");
    printf("gen_result.data = %p\n", gen_result.data);
    
    if (!gen_result.data) {
        printf("gen_result.data is NULL!\n");
        openpgp_result_free(&gen_result);
        openpgp_cleanup();
        return -1;
    }
    
    openpgp_keypair_t* keypair = (openpgp_keypair_t*)gen_result.data;
    printf("Keypair cast successful\n");
    
    if (!keypair->private_key) {
        printf("Private key is NULL!\n");
        openpgp_result_free(&gen_result);
        openpgp_cleanup();
        return -1;
    }
    
    printf("About to call openpgp_sign_data...\n");
    
    // Test sign_data function - this is where the segfault happens
    openpgp_result_t sign_result = openpgp_sign_data(
        test_message,
        keypair->private_key,
        NULL,  // No passphrase
        NULL   // Default options
    );
    
    printf("openpgp_sign_data completed\n");
    
    // Cleanup
    openpgp_result_free(&gen_result);
    openpgp_result_free(&sign_result);
    openpgp_cleanup();
    
    printf("Test completed successfully\n");
    return 0;
}