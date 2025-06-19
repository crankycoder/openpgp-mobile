#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c/include/openpgp.h"

int test_multiple_init_cycles() {
    printf("Testing multiple init/cleanup cycles...\n");
    
    for (int i = 0; i < 5; i++) {
        printf("  Cycle %d: init -> generate -> sign -> cleanup\n", i+1);
        
        // Initialize
        openpgp_result_t init_result = openpgp_init();
        if (init_result.error != 0) {
            printf("    Init failed on cycle %d\n", i+1);
            return -1;
        }
        openpgp_result_free(&init_result);
        
        // Generate key
        openpgp_result_t gen_result = openpgp_generate_key("Test User", "test@example.com", NULL);
        if (gen_result.error != 0) {
            printf("    Generate failed on cycle %d\n", i+1);
            openpgp_cleanup();
            return -1;
        }
        
        openpgp_keypair_t* keypair = (openpgp_keypair_t*)gen_result.data;
        
        // Sign with generated key
        openpgp_result_t sign_result = openpgp_sign_data("test", keypair->private_key, NULL, NULL);
        if (sign_result.error != 0) {
            printf("    Sign failed on cycle %d: %s\n", i+1, 
                   sign_result.error_message ? sign_result.error_message : "Unknown");
            openpgp_result_free(&gen_result);
            openpgp_result_free(&sign_result);
            openpgp_cleanup();
            return -1;
        }
        
        printf("    Cycle %d completed successfully\n", i+1);
        
        // Cleanup
        openpgp_result_free(&gen_result);
        openpgp_result_free(&sign_result);
        openpgp_cleanup();
    }
    
    printf("All cycles completed successfully\n");
    return 0;
}

int test_init_without_cleanup() {
    printf("Testing init without cleanup (memory leak test)...\n");
    
    for (int i = 0; i < 3; i++) {
        printf("  Init %d (no cleanup)\n", i+1);
        openpgp_result_t init_result = openpgp_init();
        if (init_result.error != 0) {
            printf("    Init %d failed\n", i+1);
            return -1;
        }
        openpgp_result_free(&init_result);
        // Deliberately not calling openpgp_cleanup()
    }
    
    printf("Testing after multiple inits without cleanup...\n");
    openpgp_result_t gen_result = openpgp_generate_key("Test User", "test@example.com", NULL);
    openpgp_keypair_t* keypair = (openpgp_keypair_t*)gen_result.data;
    
    openpgp_result_t sign_result = openpgp_sign_data("test", keypair->private_key, NULL, NULL);
    printf("Sign result after multiple inits: %d\n", sign_result.error);
    
    openpgp_result_free(&gen_result);
    openpgp_result_free(&sign_result);
    openpgp_cleanup();
    
    return 0;
}

int main() {
    printf("=== Memory Management Tests ===\n\n");
    
    printf("Test 1: Multiple init/cleanup cycles\n");
    test_multiple_init_cycles();
    
    printf("\nTest 2: Init without cleanup\n");
    test_init_without_cleanup();
    
    return 0;
}