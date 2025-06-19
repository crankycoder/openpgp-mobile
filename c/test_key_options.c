#include <stdio.h>
#include <assert.h>
#include "bridge_builder.h"
#include "bridge_reader.h"

void test_empty_key_options() {
    printf("Test 1: Empty KeyOptions\n");
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    assert(flatcc_builder_init(B) == 0);
    
    /* Build KeyOptions manually */
    assert(model_KeyOptions_start(B) == 0);
    model_KeyOptions_ref_t key_opts = model_KeyOptions_end(B);
    assert(key_opts != 0);
    
    printf("KeyOptions ref: %ld\n", (long)key_opts);
    
    /* Use it in Options */
    assert(model_Options_start(B) == 0);
    assert(model_Options_key_options_add(B, key_opts) == 0);
    model_Options_ref_t opts = model_Options_end(B);
    
    /* Build GenerateRequest */
    assert(model_GenerateRequest_start(B) == 0);
    assert(model_GenerateRequest_options_add(B, opts) == 0);
    model_GenerateRequest_ref_t req = model_GenerateRequest_end(B);
    
    assert(flatcc_builder_end_buffer(B, req) != 0);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    printf("Buffer size: %zu\n", size);
    
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    
    /* Parse and verify */
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    assert(parsed != NULL);
    
    model_Options_table_t parsed_opts = model_GenerateRequest_options(parsed);
    assert(parsed_opts != NULL);
    
    model_KeyOptions_table_t parsed_key_opts = model_Options_key_options(parsed_opts);
    assert(parsed_key_opts != NULL);
    
    printf("Success!\n");
    
    flatcc_builder_clear(B);
}

void test_key_options_with_values() {
    printf("\nTest 2: KeyOptions with values\n");
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    assert(flatcc_builder_init(B) == 0);
    
    /* Build KeyOptions with specific values */
    assert(model_KeyOptions_start(B) == 0);
    assert(model_KeyOptions_algorithm_add(B, 0) == 0);  /* RSA */
    assert(model_KeyOptions_curve_add(B, 2) == 0);      /* P256 */
    assert(model_KeyOptions_hash_add(B, 0) == 0);       /* SHA256 */
    assert(model_KeyOptions_cipher_add(B, 0) == 0);     /* AES128 */
    assert(model_KeyOptions_compression_add(B, 0) == 0); /* NONE */
    assert(model_KeyOptions_compression_level_add(B, -1) == 0);
    assert(model_KeyOptions_rsa_bits_add(B, 2048) == 0);
    model_KeyOptions_ref_t key_opts = model_KeyOptions_end(B);
    
    printf("Built KeyOptions with values\n");
    
    /* Use in Options */
    assert(model_Options_start(B) == 0);
    assert(model_Options_key_options_add(B, key_opts) == 0);
    model_Options_ref_t opts = model_Options_end(B);
    
    /* Build GenerateRequest */
    assert(model_GenerateRequest_start(B) == 0);
    assert(model_GenerateRequest_options_add(B, opts) == 0);
    model_GenerateRequest_ref_t req = model_GenerateRequest_end(B);
    
    assert(flatcc_builder_end_buffer(B, req) != 0);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    printf("Buffer size: %zu\n", size);
    
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    
    /* Parse and verify values */
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    assert(parsed != NULL);
    
    model_Options_table_t parsed_opts = model_GenerateRequest_options(parsed);
    assert(parsed_opts != NULL);
    
    model_KeyOptions_table_t parsed_key_opts = model_Options_key_options(parsed_opts);
    assert(parsed_key_opts != NULL);
    
    assert(model_KeyOptions_algorithm(parsed_key_opts) == 0);
    assert(model_KeyOptions_curve(parsed_key_opts) == 2);
    assert(model_KeyOptions_hash(parsed_key_opts) == 0);
    assert(model_KeyOptions_cipher(parsed_key_opts) == 0);
    assert(model_KeyOptions_compression(parsed_key_opts) == 0);
    assert(model_KeyOptions_compression_level(parsed_key_opts) == -1);
    assert(model_KeyOptions_rsa_bits(parsed_key_opts) == 2048);
    
    printf("All values verified!\n");
    
    flatcc_builder_clear(B);
}

int main() {
    test_empty_key_options();
    test_key_options_with_values();
    
    printf("\nAll KeyOptions tests passed!\n");
    return 0;
}