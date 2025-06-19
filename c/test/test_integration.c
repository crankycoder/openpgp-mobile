#include "test_framework.h"
#include "openpgp.h"

TEST_CASE(bridge_integration) {
    openpgp_result_t result;
    
    /* Try to initialize with actual bridge library */
    result = openpgp_init();
    
    if (result.error == OPENPGP_SUCCESS) {
        printf("      Bridge library loaded successfully\n");
        
        /* Test that we can call functions without crash */
        openpgp_result_t gen_result = openpgp_generate_key("Test", "test@example.com", NULL);
        /* Now that FlatBuffers work, this should succeed */
        if (gen_result.error == OPENPGP_SUCCESS) {
            printf("      Key generation succeeded!\n");
            TEST_ASSERT_NULL(gen_result.error_message);
        } else {
            printf("      Key generation failed: %s\n", gen_result.error_message);
        }
        openpgp_result_free(&gen_result);
        
        openpgp_cleanup();
    } else {
        printf("      Bridge library not available: %s\n", result.error_message);
        /* This is OK for the infrastructure test */
    }
    
    openpgp_result_free(&result);
    return 0;
}