#include "test_framework.h"
#include "bridge_builder.h"
#include "bridge_reader.h"
#include <string.h>

/* Test FlatBuffer serialization produces expected bytes */
TEST_CASE(flatbuffer_serialization_simple) {
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    TEST_ASSERT_EQUAL(0, flatcc_builder_init(B));
    
    /* Create a minimal GenerateRequest with just an empty Options */
    model_Options_start(B);
    model_Options_ref_t opts = model_Options_end(B);
    
    /* Create GenerateRequest */
    model_GenerateRequest_ref_t request = model_GenerateRequest_create(B, opts);
    
    /* Finish the buffer */
    TEST_ASSERT(flatcc_builder_end_buffer(B, request) != 0);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    TEST_ASSERT_NOT_NULL(buffer);
    
    /* Print the buffer for inspection */
    printf("\n      Buffer size: %zu bytes\n", size);
    printf("      Hex dump: ");
    unsigned char *bytes = (unsigned char *)buffer;
    for (size_t i = 0; i < size && i < 32; i++) {
        printf("%02x ", bytes[i]);
    }
    printf("\n");
    
    /* Expected minimal FlatBuffer structure:
     * Offset 0-3: UOffset to root table (4 bytes, little-endian)
     * The root table should point to GenerateRequest
     * GenerateRequest has one field (Options) at offset 4
     */
    
    /* Verify minimum size */
    TEST_ASSERT(size >= 8);
    
    /* Verify we can parse it back */
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    TEST_ASSERT_NOT_NULL(parsed);
    
    /* Options should be present even if empty */
    model_Options_table_t parsed_opts = model_GenerateRequest_options(parsed);
    TEST_ASSERT_NOT_NULL(parsed_opts);
    
    /* All fields should be null/default */
    TEST_ASSERT_NULL(model_Options_name(parsed_opts));
    TEST_ASSERT_NULL(model_Options_email(parsed_opts));
    TEST_ASSERT_NULL(model_Options_comment(parsed_opts));
    TEST_ASSERT_NULL(model_Options_passphrase(parsed_opts));
    
    flatcc_builder_clear(B);
    
    return 0;
}

/* Test with actual string data */
TEST_CASE(flatbuffer_serialization_with_strings) {
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    TEST_ASSERT_EQUAL(0, flatcc_builder_init(B));
    
    /* Create strings */
    flatbuffers_string_ref_t name_ref = flatbuffers_string_create_str(B, "Test");
    flatbuffers_string_ref_t email_ref = flatbuffers_string_create_str(B, "test@example.com");
    
    /* Create Options */
    model_Options_start(B);
    model_Options_name_add(B, name_ref);
    model_Options_email_add(B, email_ref);
    model_Options_ref_t opts = model_Options_end(B);
    
    /* Create GenerateRequest */
    model_GenerateRequest_ref_t request = model_GenerateRequest_create(B, opts);
    
    /* Finish the buffer */
    TEST_ASSERT(flatcc_builder_end_buffer(B, request) != 0);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    TEST_ASSERT_NOT_NULL(buffer);
    
    printf("\n      Buffer size: %zu bytes\n", size);
    printf("      First 32 bytes: ");
    unsigned char *bytes = (unsigned char *)buffer;
    for (size_t i = 0; i < size && i < 32; i++) {
        printf("%02x ", bytes[i]);
    }
    printf("\n");
    
    /* Parse it back */
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    TEST_ASSERT_NOT_NULL(parsed);
    
    model_Options_table_t parsed_opts = model_GenerateRequest_options(parsed);
    TEST_ASSERT_NOT_NULL(parsed_opts);
    
    /* Verify strings */
    TEST_ASSERT_STRING_EQUAL("Test", model_Options_name(parsed_opts));
    TEST_ASSERT_STRING_EQUAL("test@example.com", model_Options_email(parsed_opts));
    
    flatcc_builder_clear(B);
    
    return 0;
}

/* Test complete GenerateRequest matching what openpgp_generate_key creates */
TEST_CASE(flatbuffer_serialization_full_request) {
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    TEST_ASSERT_EQUAL(0, flatcc_builder_init(B));
    
    /* Create the same structure as serialize_generate_request */
    flatbuffers_string_ref_t name_ref = flatbuffers_string_create_str(B, "Test User");
    flatbuffers_string_ref_t email_ref = flatbuffers_string_create_str(B, "test@example.com");
    flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(B, "testpass");
    
    /* Create KeyOptions with defaults */
    model_KeyOptions_start(B);
    model_KeyOptions_algorithm_add(B, model_Algorithm_RSA);
    model_KeyOptions_curve_add(B, model_Curve_P256);
    model_KeyOptions_hash_add(B, model_Hash_SHA256);
    model_KeyOptions_cipher_add(B, model_Cipher_AES128);
    model_KeyOptions_compression_add(B, model_Compression_NONE);
    model_KeyOptions_compression_level_add(B, -1);
    model_KeyOptions_rsa_bits_add(B, 2048);
    model_KeyOptions_ref_t key_opts = model_KeyOptions_end(B);
    
    /* Create Options */
    model_Options_start(B);
    model_Options_name_add(B, name_ref);
    model_Options_email_add(B, email_ref);
    model_Options_passphrase_add(B, passphrase_ref);
    model_Options_key_options_add(B, key_opts);
    model_Options_ref_t opts = model_Options_end(B);
    
    /* Create GenerateRequest */
    model_GenerateRequest_ref_t request = model_GenerateRequest_create(B, opts);
    
    /* Finish the buffer */
    TEST_ASSERT(flatcc_builder_end_buffer(B, request) != 0);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    TEST_ASSERT_NOT_NULL(buffer);
    
    printf("\n      Full request buffer size: %zu bytes\n", size);
    printf("      First 64 bytes:\n      ");
    unsigned char *bytes = (unsigned char *)buffer;
    for (size_t i = 0; i < size && i < 64; i++) {
        printf("%02x ", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n      ");
    }
    printf("\n");
    
    /* Verify we can parse it completely */
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    TEST_ASSERT_NOT_NULL(parsed);
    
    model_Options_table_t parsed_opts = model_GenerateRequest_options(parsed);
    TEST_ASSERT_NOT_NULL(parsed_opts);
    
    /* Verify all fields */
    TEST_ASSERT_STRING_EQUAL("Test User", model_Options_name(parsed_opts));
    TEST_ASSERT_STRING_EQUAL("test@example.com", model_Options_email(parsed_opts));
    TEST_ASSERT_STRING_EQUAL("testpass", model_Options_passphrase(parsed_opts));
    
    /* Verify KeyOptions */
    model_KeyOptions_table_t key_options = model_Options_key_options(parsed_opts);
    TEST_ASSERT_NOT_NULL(key_options);
    TEST_ASSERT_EQUAL(0, model_KeyOptions_algorithm(key_options));
    TEST_ASSERT_EQUAL(2, model_KeyOptions_curve(key_options));
    TEST_ASSERT_EQUAL(0, model_KeyOptions_hash(key_options));
    TEST_ASSERT_EQUAL(0, model_KeyOptions_cipher(key_options));
    TEST_ASSERT_EQUAL(0, model_KeyOptions_compression(key_options));
    TEST_ASSERT_EQUAL(-1, model_KeyOptions_compression_level(key_options));
    TEST_ASSERT_EQUAL(2048, model_KeyOptions_rsa_bits(key_options));
    
    flatcc_builder_clear(B);
    
    return 0;
}