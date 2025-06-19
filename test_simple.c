#include <stdio.h>
#include <stdlib.h>
#include "c/include/openpgp.h"

int main() {
    openpgp_init();
    
    // Load fixture key
    FILE* f = fopen("c/test/gpg-test-keys/test-private-key.asc", "r");
    if (!f) {
        printf("Failed to open key file\n");
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* key = malloc(size + 1);
    fread(key, 1, size, f);
    key[size] = 0;
    fclose(f);
    
    printf("Testing openpgp_sign_data with fixture key...\n");
    openpgp_result_t result = openpgp_sign_data("test", key, NULL, NULL);
    printf("Result: error=%d\n", result.error);
    
    if (result.error == 0) {
        printf("Success! Fixture key sign_data works\n");
    } else {
        printf("Failed: %s\n", result.error_message ? result.error_message : "Unknown");
    }
    
    openpgp_result_free(&result);
    free(key);
    openpgp_cleanup();
    return 0;
}