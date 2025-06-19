#include <stdio.h>
#include <stdlib.h>
#include "c/include/openpgp.h"

int main() {
    printf("Initializing library...\n");
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != 0) {
        printf("Init failed: %d\n", init_result.error);
        return 1;
    }
    openpgp_result_free(&init_result);
    
    printf("Generating key...\n");
    openpgp_result_t gen_result = openpgp_generate_key("Test User", "test@example.com", NULL);
    
    if (gen_result.error != 0) {
        printf("Key generation failed: %d, %s\n", gen_result.error, 
               gen_result.error_message ? gen_result.error_message : "Unknown");
        openpgp_result_free(&gen_result);
        openpgp_cleanup();
        return 1;
    }
    
    openpgp_keypair_t* keypair = (openpgp_keypair_t*)gen_result.data;
    printf("Key generated successfully\n");
    printf("Private key length: %zu\n", strlen(keypair->private_key));
    
    printf("Testing openpgp_sign_data with generated key...\n");
    openpgp_result_t result = openpgp_sign_data("test", keypair->private_key, NULL, NULL);
    printf("Result: error=%d\n", result.error);
    
    if (result.error == 0) {
        printf("Success! Generated key sign_data works\n");
    } else {
        printf("Failed: %s\n", result.error_message ? result.error_message : "Unknown");
    }
    
    openpgp_result_free(&result);
    openpgp_result_free(&gen_result);
    openpgp_cleanup();
    return 0;
}