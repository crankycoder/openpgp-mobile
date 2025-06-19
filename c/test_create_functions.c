#include <stdio.h>
#include <assert.h>
#include "bridge_builder.h"
#include "bridge_reader.h"

void test_key_options_create() {
    printf("Test: KeyOptions_create function\n");
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    assert(flatcc_builder_init(B) == 0);
    
    /* Use the create function */
    model_KeyOptions_ref_t key_opts = model_KeyOptions_create(B,
        0,      /* algorithm - RSA */
        2,      /* curve - P256 */
        0,      /* hash - SHA256 */
        0,      /* cipher - AES128 */
        0,      /* compression - NONE */
        -1,     /* compression_level */
        2048    /* rsa_bits */
    );
    
    printf("KeyOptions_create returned: %ld\n", (long)key_opts);
    
    if (key_opts == 0) {
        printf("FAILED: KeyOptions_create returned 0\n");
        flatcc_builder_clear(B);
        return;
    }
    
    /* Use it in Options */
    model_Options_ref_t opts = model_Options_create(B,
        0,          /* name */
        0,          /* comment */
        0,          /* email */
        0,          /* passphrase */
        key_opts    /* key_options */
    );
    
    printf("Options_create returned: %ld\n", (long)opts);
    
    if (opts == 0) {
        printf("FAILED: Options_create returned 0\n");
        flatcc_builder_clear(B);
        return;
    }
    
    /* Create GenerateRequest */
    model_GenerateRequest_ref_t req = model_GenerateRequest_create(B, opts);
    
    printf("GenerateRequest_create returned: %ld\n", (long)req);
    
    if (req == 0) {
        printf("FAILED: GenerateRequest_create returned 0\n");
        flatcc_builder_clear(B);
        return;
    }
    
    /* Finish buffer */
    assert(flatcc_builder_end_buffer(B, req) != 0);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    printf("Buffer size: %zu\n", size);
    
    printf("Success!\n");
    
    flatcc_builder_clear(B);
}

int main() {
    test_key_options_create();
    return 0;
}