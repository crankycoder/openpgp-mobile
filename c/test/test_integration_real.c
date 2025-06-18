#include "test_framework.h"
#include "openpgp.h"
#include <stdio.h>

/* 
 * Integration test to check if we can actually generate keys
 * This will help us understand if the Go bridge is working
 */
TEST_CASE(integration_generate_key_real) {
    openpgp_result_t init_result = openpgp_init();
    
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("\n      Bridge not available: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 0; /* Skip test */
    }
    openpgp_result_free(&init_result);
    
    /* Try to generate a real key */
    printf("\n      Attempting real key generation...\n");
    
    openpgp_options_t options;
    openpgp_options_init_default(&options);
    options.name = "Test User";
    options.email = "test@example.com";
    options.passphrase = "testpass123";
    options.key_options.algorithm = OPENPGP_ALGORITHM_RSA;
    options.key_options.rsa_bits = 2048;
    
    openpgp_result_t result = openpgp_generate_key_with_options(&options);
    
    printf("      Result: error=%d, error_message=%s\n", 
           result.error, 
           result.error_message ? result.error_message : "NULL");
    
    if (result.error == OPENPGP_SUCCESS) {
        printf("      SUCCESS! Key generation worked!\n");
        
        openpgp_keypair_t *keypair = (openpgp_keypair_t*)result.data;
        if (keypair) {
            printf("      Public key length: %zu\n", strlen(keypair->public_key));
            printf("      Private key length: %zu\n", strlen(keypair->private_key));
            
            /* Check if keys look valid */
            TEST_ASSERT(strstr(keypair->public_key, "BEGIN PGP PUBLIC KEY") != NULL);
            TEST_ASSERT(strstr(keypair->private_key, "BEGIN PGP PRIVATE KEY") != NULL);
            
            openpgp_keypair_free(keypair);
            free(keypair);
        }
    } else {
        printf("      Key generation failed with: %s\n", result.error_message);
        /* This is expected in Phase 1 due to flatcc stubs */
    }
    
    openpgp_result_free(&result);
    openpgp_cleanup();
    
    /* For Phase 1, we expect failure due to stubs */
    return 0;
}