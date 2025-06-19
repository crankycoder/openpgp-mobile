#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bridge_builder.h"
#include "bridge_reader.h"

void test_simple() {
    printf("Test 1: Simple FlatBuffer\n");
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    assert(flatcc_builder_init(B) == 0);
    
    /* Create empty options */
    model_Options_ref_t opts = model_Options_create(B, 0, 0, 0, 0, 0);
    
    /* Create GenerateRequest */
    model_GenerateRequest_ref_t req = model_GenerateRequest_create(B, opts);
    
    /* Finish buffer */
    assert(flatcc_builder_end_buffer(B, req) != 0);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    printf("Buffer size: %zu\n", size);
    
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    assert(buffer != NULL);
    
    /* Parse it back */
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    assert(parsed != NULL);
    
    printf("Parse successful!\n");
    
    flatcc_builder_clear(B);
}

void test_with_data() {
    printf("\nTest 2: FlatBuffer with data\n");
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    assert(flatcc_builder_init(B) == 0);
    
    /* Create strings */
    flatbuffers_string_ref_t name = flatbuffers_string_create_str(B, "Test User");
    flatbuffers_string_ref_t email = flatbuffers_string_create_str(B, "test@example.com");
    
    /* Create empty KeyOptions first */
    model_KeyOptions_ref_t key_opts = model_KeyOptions_create(B,
        0,  /* RSA */
        2,  /* P256 */
        0,  /* SHA256 */
        0,  /* AES128 */
        0,  /* NONE */
        -1,
        2048
    );
    
    /* Create Options */
    model_Options_ref_t opts = model_Options_create(B, name, 0, email, 0, key_opts);
    
    /* Create GenerateRequest */
    model_GenerateRequest_ref_t req = model_GenerateRequest_create(B, opts);
    
    /* Finish buffer */
    assert(flatcc_builder_end_buffer(B, req) != 0);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    printf("Buffer size: %zu\n", size);
    
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    
    /* Parse and verify */
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    assert(parsed != NULL);
    
    model_Options_table_t opts_parsed = model_GenerateRequest_options(parsed);
    assert(opts_parsed != NULL);
    
    const char *parsed_name = model_Options_name(opts_parsed);
    const char *parsed_email = model_Options_email(opts_parsed);
    
    printf("Name: %s\n", parsed_name);
    printf("Email: %s\n", parsed_email);
    
    assert(strcmp(parsed_name, "Test User") == 0);
    assert(strcmp(parsed_email, "test@example.com") == 0);
    
    printf("Verification successful!\n");
    
    flatcc_builder_clear(B);
}

int main() {
    test_simple();
    test_with_data();
    
    printf("\nAll tests passed!\n");
    return 0;
}