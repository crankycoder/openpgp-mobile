#include <stdio.h>
#include <string.h>
#include "bridge_builder.h"
#include "bridge_reader.h"

int main() {
    printf("Running FlatBuffer serialization tests...\n\n");
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    // Test 1: Empty request
    printf("Test 1: Empty GenerateRequest\n");
    if (flatcc_builder_init(B)) {
        printf("FAILED: builder init\n");
        return 1;
    }
    
    model_Options_ref_t opts = model_Options_create(B, 0, 0, 0, 0, 0);
    model_GenerateRequest_ref_t req = model_GenerateRequest_create(B, opts);
    
    if (!flatcc_builder_end_buffer(B, req)) {
        printf("FAILED: end_buffer\n");
        return 1;
    }
    
    size_t size = flatcc_builder_get_buffer_size(B);
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    
    printf("Buffer size: %zu bytes\n", size);
    printf("Hex: ");
    for (size_t i = 0; i < size && i < 16; i++) {
        printf("%02x ", ((unsigned char*)buffer)[i]);
    }
    printf("\n");
    
    // Verify we can parse it
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    if (parsed) {
        printf("Parse: SUCCESS\n");
    } else {
        printf("Parse: FAILED\n");
    }
    
    flatcc_builder_clear(B);
    
    // Test 2: Full request
    printf("\nTest 2: Full GenerateRequest\n");
    if (flatcc_builder_init(B)) {
        printf("FAILED: builder init\n");
        return 1;
    }
    
    flatbuffers_string_ref_t name_ref = flatbuffers_string_create_str(B, "Test User");
    flatbuffers_string_ref_t email_ref = flatbuffers_string_create_str(B, "test@example.com");
    flatbuffers_string_ref_t pass_ref = flatbuffers_string_create_str(B, "testpass");
    
    model_KeyOptions_ref_t key_opts = model_KeyOptions_create(B,
        0, 2, 0, 0, 0, -1, 2048);
    
    opts = model_Options_create(B,
        name_ref, 0, email_ref, pass_ref, key_opts);
    
    req = model_GenerateRequest_create(B, opts);
    
    if (!flatcc_builder_end_buffer(B, req)) {
        printf("FAILED: end_buffer\n");
        return 1;
    }
    
    size = flatcc_builder_get_buffer_size(B);
    buffer = flatcc_builder_get_direct_buffer(B, &size);
    
    printf("Buffer size: %zu bytes\n", size);
    printf("First 32 bytes:\n");
    for (size_t i = 0; i < size && i < 32; i++) {
        printf("%02x ", ((unsigned char*)buffer)[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
    
    // Parse and verify
    parsed = model_GenerateRequest_as_root(buffer);
    if (parsed) {
        printf("Parse: SUCCESS\n");
        model_Options_table_t opts_t = model_GenerateRequest_options(parsed);
        if (opts_t) {
            const char *name = model_Options_name(opts_t);
            const char *email = model_Options_email(opts_t);
            printf("Name: %s\n", name ? name : "(null)");
            printf("Email: %s\n", email ? email : "(null)");
        }
    } else {
        printf("Parse: FAILED\n");
    }
    
    flatcc_builder_clear(B);
    
    printf("\nAll tests completed\n");
    return 0;
}