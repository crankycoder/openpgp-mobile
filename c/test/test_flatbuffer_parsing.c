#include "test_framework.h"
#include "openpgp.h"
#include "bridge_builder.h"
#include "bridge_reader.h"
#include <string.h>

/* Test that we can create and parse FlatBuffer requests */
TEST_CASE(create_generate_request) {
    /* Initialize library */
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("      Skipping - bridge not available\n");
        openpgp_result_free(&init_result);
        return 0;
    }
    openpgp_result_free(&init_result);
    
    /* Create a FlatBuffer builder */
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    TEST_ASSERT_EQUAL(0, flatcc_builder_init(B));
    
    /* Create a simple GenerateRequest */
    flatbuffers_string_ref_t email_ref = flatbuffers_string_create_str(B, "test@example.com");
    flatbuffers_string_ref_t name_ref = flatbuffers_string_create_str(B, "Test User");
    
    /* Create KeyOptions with defaults */
    model_KeyOptions_ref_t key_opts = model_KeyOptions_create(B,
        model_Algorithm_RSA,      /* algorithm */
        model_Curve_P256,        /* curve */
        model_Hash_SHA256,       /* hash */
        model_Cipher_AES128,     /* cipher */
        model_Compression_NONE,  /* compression */
        -1,                     /* compression_level */
        2048                    /* rsa_bits */
    );
    
    /* Create Options */
    model_Options_ref_t opts = model_Options_create(B,
        name_ref,       /* name */
        0,             /* comment */
        email_ref,     /* email */
        0,             /* passphrase */
        key_opts       /* key_options */
    );
    
    /* Create GenerateRequest */
    model_GenerateRequest_ref_t request = model_GenerateRequest_create(B, opts);
    
    /* Finish the buffer */
    TEST_ASSERT(flatcc_builder_end_buffer(B, request) != 0);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    TEST_ASSERT(size > 0);
    
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    TEST_ASSERT_NOT_NULL(buffer);
    
    /* Verify we can parse it back */
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    TEST_ASSERT_NOT_NULL(parsed);
    
    model_Options_table_t parsed_opts = model_GenerateRequest_options(parsed);
    TEST_ASSERT_NOT_NULL(parsed_opts);
    
    TEST_ASSERT_STRING_EQUAL("test@example.com", model_Options_email(parsed_opts));
    TEST_ASSERT_STRING_EQUAL("Test User", model_Options_name(parsed_opts));
    
    flatcc_builder_clear(B);
    openpgp_cleanup();
    
    return 0;
}

/* Test creating and parsing KeyPairResponse */
TEST_CASE(parse_keypair_response) {
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    TEST_ASSERT_EQUAL(0, flatcc_builder_init(B));
    
    /* Create a mock KeyPairResponse */
    const char *public_key = "-----BEGIN PGP PUBLIC KEY BLOCK-----\ntest\n-----END PGP PUBLIC KEY BLOCK-----";
    const char *private_key = "-----BEGIN PGP PRIVATE KEY BLOCK-----\ntest\n-----END PGP PRIVATE KEY BLOCK-----";
    
    flatbuffers_string_ref_t pub_ref = flatbuffers_string_create_str(B, public_key);
    flatbuffers_string_ref_t priv_ref = flatbuffers_string_create_str(B, private_key);
    
    model_KeyPair_ref_t keypair = model_KeyPair_create(B, pub_ref, priv_ref);
    model_KeyPairResponse_create_as_root(B, keypair, 0);
    
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(B, &size);
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT(size > 0);
    
    /* Parse the response */
    model_KeyPairResponse_table_t response = model_KeyPairResponse_as_root(buffer);
    TEST_ASSERT_NOT_NULL(response);
    
    /* Check for error */
    flatbuffers_string_t error = model_KeyPairResponse_error(response);
    TEST_ASSERT_NULL(error);
    
    /* Get keypair */
    model_KeyPair_table_t kp = model_KeyPairResponse_output(response);
    TEST_ASSERT_NOT_NULL(kp);
    
    /* Verify keys */
    TEST_ASSERT_STRING_EQUAL(public_key, model_KeyPair_public_key(kp));
    TEST_ASSERT_STRING_EQUAL(private_key, model_KeyPair_private_key(kp));
    
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(B);
    
    return 0;
}