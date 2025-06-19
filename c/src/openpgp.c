#include "openpgp.h"
#include "bridge_builder.h" 
#include "bridge_reader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

/* Bridge function type */
typedef struct {
    void* message;
    int size;
    char* error;
} BytesReturn;

typedef BytesReturn* (*OpenPGPBridgeCall_fn)(char* name, void* payload, int payloadSize);

/* Global state */
static struct {
    bool initialized;
    void *bridge_handle;
    OpenPGPBridgeCall_fn bridge_call;
} g_openpgp = {0};

/* Internal helper functions */
static openpgp_result_t create_error_result(openpgp_error_t error, const char *message);
static openpgp_result_t create_success_result(void *data, size_t data_size);
static char *duplicate_string(const char *str);
static openpgp_result_t serialize_generate_request(const openpgp_options_t *options, void **buffer, size_t *buffer_size);
static openpgp_result_t parse_keypair_response(const void *response_data, size_t response_size);

/*
 * Library Initialization and Cleanup
 */

openpgp_result_t openpgp_init(void) {
    if (g_openpgp.initialized) {
        return create_success_result(NULL, 0);
    }

    /* Load the bridge library */
    g_openpgp.bridge_handle = dlopen("libopenpgp_bridge.so", RTLD_LAZY);
    if (!g_openpgp.bridge_handle) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, 
                                 "Failed to load libopenpgp_bridge.so");
    }

    /* Get function pointers */
    g_openpgp.bridge_call = dlsym(g_openpgp.bridge_handle, "OpenPGPBridgeCall");
    if (!g_openpgp.bridge_call) {
        dlclose(g_openpgp.bridge_handle);
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL,
                                 "Failed to find OpenPGPBridgeCall symbol");
    }

    /* OpenPGPFreeBytesReturn is not available in current bridge, we'll manage memory ourselves */

    g_openpgp.initialized = true;
    return create_success_result(NULL, 0);
}

void openpgp_cleanup(void) {
    if (!g_openpgp.initialized) {
        return;
    }

    if (g_openpgp.bridge_handle) {
        dlclose(g_openpgp.bridge_handle);
        g_openpgp.bridge_handle = NULL;
    }

    g_openpgp.bridge_call = NULL;
    g_openpgp.initialized = false;
}/*
 * Memory Management
 */

void openpgp_result_free(openpgp_result_t *result) {
    if (!result) return;
    
    free(result->error_message);
    free(result->data);
    memset(result, 0, sizeof(*result));
}

void openpgp_keypair_free(openpgp_keypair_t *keypair) {
    if (!keypair) return;
    
    free(keypair->public_key);
    free(keypair->private_key);
    memset(keypair, 0, sizeof(*keypair));
}

/*
 * Key Generation (placeholder implementations)
 */

openpgp_result_t openpgp_generate_key(const char *name, const char *email, const char *passphrase) {
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized. Call openpgp_init() first.");
    }

    openpgp_options_t options;
    openpgp_options_init_default(&options);
    options.name = name;
    options.email = email;
    options.passphrase = passphrase;
    
    return openpgp_generate_key_with_options(&options);
}

openpgp_result_t openpgp_generate_key_with_options(const openpgp_options_t *options) {
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized. Call openpgp_init() first.");
    }

    if (!options) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Options cannot be NULL");
    }

    /* Serialize the request using FlatBuffers */
    void *request_buffer = NULL;
    size_t request_size = 0;
    openpgp_result_t serialize_result = serialize_generate_request(options, &request_buffer, &request_size);
    if (serialize_result.error != OPENPGP_SUCCESS) {
        return serialize_result;
    }
    
    /* Debug output removed - was too verbose */

    /* Call the bridge */
    BytesReturn *bridge_result = g_openpgp.bridge_call(
        "generate", 
        request_buffer, 
        (int)request_size
    );
    
    free(request_buffer);

    if (!bridge_result) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call returned NULL");
    }

    openpgp_result_t result;
    if (bridge_result->error) {
        result = create_error_result(OPENPGP_ERROR_KEY_GENERATION_FAILED, bridge_result->error);
    } else if (!bridge_result->message || bridge_result->size == 0) {
        result = create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge returned empty response");
    } else {
        result = parse_keypair_response(bridge_result->message, bridge_result->size);
    }

    /* Free bridge result memory */
    if (bridge_result->message) {
        free(bridge_result->message);
    }
    if (bridge_result->error) {
        free(bridge_result->error);
    }
    free(bridge_result);

    return result;
}

/*
 * Helper Functions
 */

const char *openpgp_error_string(openpgp_error_t error) {
    switch (error) {
        case OPENPGP_SUCCESS: return "Success";
        case OPENPGP_ERROR_INVALID_INPUT: return "Invalid input";
        case OPENPGP_ERROR_ENCRYPTION_FAILED: return "Encryption failed";
        case OPENPGP_ERROR_DECRYPTION_FAILED: return "Decryption failed";
        case OPENPGP_ERROR_SIGNING_FAILED: return "Signing failed";
        case OPENPGP_ERROR_VERIFICATION_FAILED: return "Verification failed";
        case OPENPGP_ERROR_KEY_GENERATION_FAILED: return "Key generation failed";
        case OPENPGP_ERROR_MEMORY_ALLOCATION: return "Memory allocation failed";
        case OPENPGP_ERROR_SERIALIZATION: return "Serialization failed";
        case OPENPGP_ERROR_BRIDGE_CALL: return "Bridge call failed";
        case OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED: return "Library not initialized";
        case OPENPGP_ERROR_UNKNOWN:
        default: return "Unknown error";
    }
}void openpgp_options_init_default(openpgp_options_t *options) {
    if (!options) return;
    
    memset(options, 0, sizeof(*options));
    openpgp_key_options_init_default(&options->key_options);
}

void openpgp_key_options_init_default(openpgp_key_options_t *key_options) {
    if (!key_options) return;
    
    memset(key_options, 0, sizeof(*key_options));
    key_options->algorithm = OPENPGP_ALGORITHM_RSA;
    key_options->curve = OPENPGP_CURVE_P256;
    key_options->hash = OPENPGP_HASH_SHA256;
    key_options->cipher = OPENPGP_CIPHER_AES128;
    key_options->compression = OPENPGP_COMPRESSION_NONE;
    key_options->compression_level = -1;  /* Default */
    key_options->rsa_bits = 2048;
}

/*
 * Internal helper functions
 */

static openpgp_result_t create_error_result(openpgp_error_t error, const char *message) {
    openpgp_result_t result = {0};
    result.error = error;
    result.error_message = duplicate_string(message);
    result.data = NULL;
    result.data_size = 0;
    return result;
}

static openpgp_result_t create_success_result(void *data, size_t data_size) {
    openpgp_result_t result = {0};
    result.error = OPENPGP_SUCCESS;
    result.error_message = NULL;
    result.data = data;
    result.data_size = data_size;
    return result;
}

static char *duplicate_string(const char *str) {
    if (!str) return NULL;
    
    size_t len = strlen(str) + 1;
    char *dup = malloc(len);
    if (dup) {
        memcpy(dup, str, len);
    }
    return dup;
}

/* Internal helper to serialize generate request using FlatBuffers */
static openpgp_result_t serialize_generate_request(const openpgp_options_t *options, void **buffer, size_t *buffer_size) {
    /* Create FlatBuffer builder */
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    if (flatcc_builder_init(B)) {
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to initialize FlatBuffer builder");
    }
    
    /* Create string references */
    flatbuffers_string_ref_t name_ref = 0;
    flatbuffers_string_ref_t email_ref = 0; 
    flatbuffers_string_ref_t comment_ref = 0;
    flatbuffers_string_ref_t passphrase_ref = 0;
    
    if (options->name) {
        name_ref = flatbuffers_string_create_str(B, options->name);
    }
    if (options->email) {
        email_ref = flatbuffers_string_create_str(B, options->email);
    }
    if (options->comment) {
        comment_ref = flatbuffers_string_create_str(B, options->comment);
    }
    if (options->passphrase) {
        passphrase_ref = flatbuffers_string_create_str(B, options->passphrase);
    }
    
    /* Create KeyOptions */
    model_KeyOptions_start(B);
    model_KeyOptions_algorithm_add(B, (model_Algorithm_enum_t)options->key_options.algorithm);
    model_KeyOptions_curve_add(B, (model_Curve_enum_t)options->key_options.curve);
    model_KeyOptions_hash_add(B, (model_Hash_enum_t)options->key_options.hash);
    model_KeyOptions_cipher_add(B, (model_Cipher_enum_t)options->key_options.cipher);
    model_KeyOptions_compression_add(B, (model_Compression_enum_t)options->key_options.compression);
    model_KeyOptions_compression_level_add(B, options->key_options.compression_level);
    model_KeyOptions_rsa_bits_add(B, options->key_options.rsa_bits);
    model_KeyOptions_ref_t key_options_ref = model_KeyOptions_end(B);
    
    /* Create Options */
    model_Options_start(B);
    if (name_ref) model_Options_name_add(B, name_ref);
    if (comment_ref) model_Options_comment_add(B, comment_ref);
    if (email_ref) model_Options_email_add(B, email_ref);
    if (passphrase_ref) model_Options_passphrase_add(B, passphrase_ref);
    if (key_options_ref) model_Options_key_options_add(B, key_options_ref);
    model_Options_ref_t options_ref = model_Options_end(B);
    
    /* Create GenerateRequest */
    model_GenerateRequest_ref_t request_ref = model_GenerateRequest_create(B, options_ref);
    
    /* Finish the buffer */
    if (!flatcc_builder_end_buffer(B, request_ref)) {
        flatcc_builder_clear(B);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to create FlatBuffer");
    }
    
    /* Get buffer size and data */
    *buffer_size = flatcc_builder_get_buffer_size(B);
    void *data = malloc(*buffer_size);
    if (!data) {
        flatcc_builder_clear(B);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to allocate buffer");
    }
    
    /* Copy the buffer data */
    void *builder_buffer = flatcc_builder_get_direct_buffer(B, buffer_size);
    memcpy(data, builder_buffer, *buffer_size);
    *buffer = data;
    
    flatcc_builder_clear(B);
    return create_success_result(NULL, 0);
}

/* Helper to parse keypair response using FlatBuffers */
static openpgp_result_t parse_keypair_response(const void *response_data, size_t response_size) {
    if (!response_data || response_size == 0) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "No response data");
    }
    
    /* Parse FlatBuffer response as KeyPairResponse */
    model_KeyPairResponse_table_t response = model_KeyPairResponse_as_root(response_data);
    if (!response) {
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Invalid FlatBuffer response");
    }
    
    /* Check for error in response */
    flatbuffers_string_t error_str = model_KeyPairResponse_error(response);
    if (error_str && flatbuffers_string_len(error_str) > 0) {
        char *error_msg = duplicate_string(error_str);
        return create_error_result(OPENPGP_ERROR_KEY_GENERATION_FAILED, error_msg);
    }
    
    /* Get output KeyPair */
    model_KeyPair_table_t keypair_table = model_KeyPairResponse_output(response);
    if (!keypair_table) {
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No keypair in response");
    }
    
    /* Extract public and private keys */
    flatbuffers_string_t public_key_str = model_KeyPair_public_key(keypair_table);
    flatbuffers_string_t private_key_str = model_KeyPair_private_key(keypair_table);
    
    if (!public_key_str || !private_key_str) {
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Missing keys in response");
    }
    
    /* Create C keypair structure */
    openpgp_keypair_t *keypair = malloc(sizeof(openpgp_keypair_t));
    if (!keypair) {
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to allocate keypair");
    }
    
    keypair->public_key = duplicate_string(public_key_str);
    keypair->private_key = duplicate_string(private_key_str);
    
    if (!keypair->public_key || !keypair->private_key) {
        openpgp_keypair_free(keypair);
        free(keypair);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to copy key strings");
    }
    
    return create_success_result(keypair, sizeof(openpgp_keypair_t));
}

/*
 * Key Operations
 */

openpgp_result_t openpgp_convert_private_to_public(const char *private_key) {
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }
    
    if (!private_key || strlen(private_key) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Private key is required");
    }
    
    /* Build ConvertPrivateKeyToPublicKeyRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually using start/add/end pattern */
    model_ConvertPrivateKeyToPublicKeyRequest_start_as_root(&builder);
    
    /* Add private_key field */
    flatbuffers_string_ref_t private_key_ref = flatbuffers_string_create_str(&builder, private_key);
    model_ConvertPrivateKeyToPublicKeyRequest_private_key_add(&builder, private_key_ref);
    
    /* End the request */
    model_ConvertPrivateKeyToPublicKeyRequest_end_as_root(&builder);
    
    /* Get buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, 
                                 "Failed to serialize ConvertPrivateKeyToPublicKeyRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("convertPrivateKeyToPublicKey", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse StringResponse */
    model_StringResponse_table_t string_response = model_StringResponse_as_root(response->message);
    if (!string_response) {
        /* Free response if needed */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to parse StringResponse");
    }
    
    /* Check for error in response */
    flatbuffers_string_t error = model_StringResponse_error(string_response);
    if (error && strlen(error) > 0) {
        char *error_copy = duplicate_string(error);
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, error_copy);
    }
    
    /* Get the public key */
    flatbuffers_string_t public_key_str = model_StringResponse_output(string_response);
    if (!public_key_str) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No public key in response");
    }
    
    /* Duplicate the public key string */
    char *public_key_copy = duplicate_string(public_key_str);
    if (!public_key_copy) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to copy public key");
    }
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(public_key_copy, strlen(public_key_copy) + 1);
}

openpgp_result_t openpgp_get_public_key_metadata(const char *public_key) {
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }
    
    if (!public_key || strlen(public_key) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Public key is required");
    }
    
    /* Build GetPublicKeyMetadataRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_GetPublicKeyMetadataRequest_start_as_root(&builder);
    
    /* Add public_key field */
    flatbuffers_string_ref_t public_key_ref = flatbuffers_string_create_str(&builder, public_key);
    model_GetPublicKeyMetadataRequest_public_key_add(&builder, public_key_ref);
    
    /* End the request */
    model_GetPublicKeyMetadataRequest_end_as_root(&builder);
    
    /* Get buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, 
                                 "Failed to serialize GetPublicKeyMetadataRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("getPublicKeyMetadata", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse PublicKeyMetadataResponse */
    model_PublicKeyMetadataResponse_table_t metadata_response = model_PublicKeyMetadataResponse_as_root(response->message);
    if (!metadata_response) {
        /* Free response if needed */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to parse PublicKeyMetadataResponse");
    }
    
    /* Check for error in response */
    flatbuffers_string_t error = model_PublicKeyMetadataResponse_error(metadata_response);
    if (error && strlen(error) > 0) {
        char *error_copy = duplicate_string(error);
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, error_copy);
    }
    
    /* Get the metadata */
    model_PublicKeyMetadata_table_t fb_metadata = model_PublicKeyMetadataResponse_output(metadata_response);
    if (!fb_metadata) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No metadata in response");
    }
    
    /* Allocate C metadata structure */
    openpgp_public_key_metadata_t *metadata = calloc(1, sizeof(openpgp_public_key_metadata_t));
    if (!metadata) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to allocate metadata");
    }
    
    /* Extract fields from FlatBuffer metadata */
    metadata->algorithm = duplicate_string(model_PublicKeyMetadata_algorithm(fb_metadata));
    metadata->key_id = duplicate_string(model_PublicKeyMetadata_key_id(fb_metadata));
    metadata->key_id_short = duplicate_string(model_PublicKeyMetadata_key_id_short(fb_metadata));
    metadata->creation_time = duplicate_string(model_PublicKeyMetadata_creation_time(fb_metadata));
    metadata->fingerprint = duplicate_string(model_PublicKeyMetadata_fingerprint(fb_metadata));
    metadata->key_id_numeric = duplicate_string(model_PublicKeyMetadata_key_id_numeric(fb_metadata));
    metadata->is_sub_key = model_PublicKeyMetadata_is_sub_key(fb_metadata);
    metadata->can_sign = model_PublicKeyMetadata_can_sign(fb_metadata);
    metadata->can_encrypt = model_PublicKeyMetadata_can_encrypt(fb_metadata);
    
    /* Extract identities */
    model_Identity_vec_t identities = model_PublicKeyMetadata_identities(fb_metadata);
    if (identities) {
        size_t count = model_Identity_vec_len(identities);
        if (count > 0) {
            metadata->identities = calloc(count, sizeof(openpgp_identity_t));
            if (metadata->identities) {
                metadata->identities_count = count;
                for (size_t i = 0; i < count; i++) {
                    model_Identity_table_t fb_identity = model_Identity_vec_at(identities, i);
                    if (fb_identity) {
                        metadata->identities[i].id = duplicate_string(model_Identity_id(fb_identity));
                        metadata->identities[i].name = duplicate_string(model_Identity_name(fb_identity));
                        metadata->identities[i].email = duplicate_string(model_Identity_email(fb_identity));
                        metadata->identities[i].comment = duplicate_string(model_Identity_comment(fb_identity));
                    }
                }
            }
        }
    }
    
    /* Note: Subkeys extraction would be similar but more complex due to nested structure */
    /* For now, we'll leave sub_keys as NULL */
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(metadata, sizeof(openpgp_public_key_metadata_t));
}

openpgp_result_t openpgp_get_private_key_metadata(const char *private_key) {
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }
    
    if (!private_key || strlen(private_key) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Private key is required");
    }
    
    /* Build GetPrivateKeyMetadataRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_GetPrivateKeyMetadataRequest_start_as_root(&builder);
    
    /* Add private_key field */
    flatbuffers_string_ref_t private_key_ref = flatbuffers_string_create_str(&builder, private_key);
    model_GetPrivateKeyMetadataRequest_private_key_add(&builder, private_key_ref);
    
    /* End the request */
    model_GetPrivateKeyMetadataRequest_end_as_root(&builder);
    
    /* Get buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, 
                                 "Failed to serialize GetPrivateKeyMetadataRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("getPrivateKeyMetadata", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse PrivateKeyMetadataResponse */
    model_PrivateKeyMetadataResponse_table_t metadata_response = model_PrivateKeyMetadataResponse_as_root(response->message);
    if (!metadata_response) {
        /* Free response if needed */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to parse PrivateKeyMetadataResponse");
    }
    
    /* Check for error in response */
    flatbuffers_string_t error = model_PrivateKeyMetadataResponse_error(metadata_response);
    if (error && strlen(error) > 0) {
        char *error_copy = duplicate_string(error);
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, error_copy);
    }
    
    /* Get the metadata */
    model_PrivateKeyMetadata_table_t fb_metadata = model_PrivateKeyMetadataResponse_output(metadata_response);
    if (!fb_metadata) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No metadata in response");
    }
    
    /* Allocate C metadata structure */
    openpgp_private_key_metadata_t *metadata = calloc(1, sizeof(openpgp_private_key_metadata_t));
    if (!metadata) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to allocate metadata");
    }
    
    /* Extract fields from FlatBuffer metadata */
    metadata->key_id = duplicate_string(model_PrivateKeyMetadata_key_id(fb_metadata));
    metadata->key_id_short = duplicate_string(model_PrivateKeyMetadata_key_id_short(fb_metadata));
    metadata->creation_time = duplicate_string(model_PrivateKeyMetadata_creation_time(fb_metadata));
    metadata->fingerprint = duplicate_string(model_PrivateKeyMetadata_fingerprint(fb_metadata));
    metadata->key_id_numeric = duplicate_string(model_PrivateKeyMetadata_key_id_numeric(fb_metadata));
    metadata->is_sub_key = model_PrivateKeyMetadata_is_sub_key(fb_metadata);
    metadata->encrypted = model_PrivateKeyMetadata_encrypted(fb_metadata);
    metadata->can_sign = model_PrivateKeyMetadata_can_sign(fb_metadata);
    
    /* Extract identities */
    model_Identity_vec_t identities = model_PrivateKeyMetadata_identities(fb_metadata);
    if (identities) {
        size_t count = model_Identity_vec_len(identities);
        if (count > 0) {
            metadata->identities = calloc(count, sizeof(openpgp_identity_t));
            if (metadata->identities) {
                metadata->identities_count = count;
                for (size_t i = 0; i < count; i++) {
                    model_Identity_table_t fb_identity = model_Identity_vec_at(identities, i);
                    if (fb_identity) {
                        metadata->identities[i].id = duplicate_string(model_Identity_id(fb_identity));
                        metadata->identities[i].name = duplicate_string(model_Identity_name(fb_identity));
                        metadata->identities[i].email = duplicate_string(model_Identity_email(fb_identity));
                        metadata->identities[i].comment = duplicate_string(model_Identity_comment(fb_identity));
                    }
                }
            }
        }
    }
    
    /* Note: Subkeys extraction would be similar but more complex due to nested structure */
    /* For now, we'll leave sub_keys as NULL */
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(metadata, sizeof(openpgp_private_key_metadata_t));
}

/*
 * Memory management for metadata structures
 */

void openpgp_public_key_metadata_free(openpgp_public_key_metadata_t *metadata) {
    if (!metadata) return;
    
    /* Free string fields */
    free(metadata->algorithm);
    free(metadata->key_id);
    free(metadata->key_id_short);
    free(metadata->creation_time);
    free(metadata->fingerprint);
    free(metadata->key_id_numeric);
    
    /* Free identities */
    if (metadata->identities) {
        for (size_t i = 0; i < metadata->identities_count; i++) {
            free(metadata->identities[i].id);
            free(metadata->identities[i].name);
            free(metadata->identities[i].email);
            free(metadata->identities[i].comment);
        }
        free(metadata->identities);
    }
    
    /* Free subkeys if implemented */
    /* TODO: Implement recursive freeing of subkeys */
    
    /* Zero out and free the structure */
    memset(metadata, 0, sizeof(*metadata));
    free(metadata);
}

void openpgp_private_key_metadata_free(openpgp_private_key_metadata_t *metadata) {
    if (!metadata) return;
    
    /* Free string fields */
    free(metadata->key_id);
    free(metadata->key_id_short);
    free(metadata->creation_time);
    free(metadata->fingerprint);
    free(metadata->key_id_numeric);
    
    /* Free identities */
    if (metadata->identities) {
        for (size_t i = 0; i < metadata->identities_count; i++) {
            free(metadata->identities[i].id);
            free(metadata->identities[i].name);
            free(metadata->identities[i].email);
            free(metadata->identities[i].comment);
        }
        free(metadata->identities);
    }
    
    /* Free subkeys if implemented */
    /* TODO: Implement recursive freeing of subkeys */
    
    /* Zero out and free the structure */
    memset(metadata, 0, sizeof(*metadata));
    free(metadata);
}

/*
 * Symmetric Encryption Operations
 */

/* Helper function to build KeyOptions FlatBuffer */
static model_KeyOptions_ref_t build_key_options(flatcc_builder_t *B, const openpgp_key_options_t *options) {
    if (!options) {
        return 0; /* Return null reference for optional field */
    }
    
    model_KeyOptions_start(B);
    model_KeyOptions_algorithm_add(B, (model_Algorithm_enum_t)options->algorithm);
    model_KeyOptions_curve_add(B, (model_Curve_enum_t)options->curve);
    model_KeyOptions_hash_add(B, (model_Hash_enum_t)options->hash);
    model_KeyOptions_cipher_add(B, (model_Cipher_enum_t)options->cipher);
    model_KeyOptions_compression_add(B, (model_Compression_enum_t)options->compression);
    model_KeyOptions_compression_level_add(B, options->compression_level);
    model_KeyOptions_rsa_bits_add(B, options->rsa_bits);
    return model_KeyOptions_end(B);
}

/* Helper function to build FileHints FlatBuffer */
static model_FileHints_ref_t build_file_hints(flatcc_builder_t *B, const openpgp_file_hints_t *hints) {
    if (!hints) {
        return 0; /* Return null reference for optional field */
    }
    
    model_FileHints_start(B);
    model_FileHints_is_binary_add(B, hints->is_binary);
    
    if (hints->file_name) {
        flatbuffers_string_ref_t file_name_ref = flatbuffers_string_create_str(B, hints->file_name);
        model_FileHints_file_name_add(B, file_name_ref);
    }
    
    if (hints->mod_time) {
        flatbuffers_string_ref_t mod_time_ref = flatbuffers_string_create_str(B, hints->mod_time);
        model_FileHints_mod_time_add(B, mod_time_ref);
    }
    
    return model_FileHints_end(B);
}

openpgp_result_t openpgp_encrypt_symmetric(const char *message, const char *passphrase, 
                                          const openpgp_file_hints_t *file_hints,
                                          const openpgp_key_options_t *options) {
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }
    
    if (!message || strlen(message) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Message is required");
    }
    
    if (!passphrase || strlen(passphrase) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Passphrase is required");
    }
    
    /* Build EncryptSymmetricRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_EncryptSymmetricRequest_start_as_root(&builder);
    
    /* Add message field */
    flatbuffers_string_ref_t message_ref = flatbuffers_string_create_str(&builder, message);
    model_EncryptSymmetricRequest_message_add(&builder, message_ref);
    
    /* Add passphrase field */
    flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(&builder, passphrase);
    model_EncryptSymmetricRequest_passphrase_add(&builder, passphrase_ref);
    
    /* Add options field if provided */
    model_KeyOptions_ref_t options_ref = build_key_options(&builder, options);
    if (options_ref) {
        model_EncryptSymmetricRequest_options_add(&builder, options_ref);
    }
    
    /* Add file_hints field if provided */
    model_FileHints_ref_t file_hints_ref = build_file_hints(&builder, file_hints);
    if (file_hints_ref) {
        model_EncryptSymmetricRequest_file_hints_add(&builder, file_hints_ref);
    }
    
    /* End the request */
    model_EncryptSymmetricRequest_end_as_root(&builder);
    
    /* Get buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, 
                                 "Failed to serialize EncryptSymmetricRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("encryptSymmetric", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse StringResponse */
    model_StringResponse_table_t string_response = model_StringResponse_as_root(response->message);
    if (!string_response) {
        /* Free response if needed */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to parse StringResponse");
    }
    
    /* Check for error in response */
    flatbuffers_string_t error = model_StringResponse_error(string_response);
    if (error && strlen(error) > 0) {
        char *error_copy = duplicate_string(error);
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_ENCRYPTION_FAILED, error_copy);
    }
    
    /* Get the encrypted message */
    flatbuffers_string_t encrypted_str = model_StringResponse_output(string_response);
    if (!encrypted_str) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No encrypted message in response");
    }
    
    /* Duplicate the encrypted message string */
    char *encrypted_copy = duplicate_string(encrypted_str);
    if (!encrypted_copy) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to copy encrypted message");
    }
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(encrypted_copy, strlen(encrypted_copy) + 1);
}

openpgp_result_t openpgp_decrypt_symmetric(const char *message, const char *passphrase,
                                          const openpgp_key_options_t *options) {
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }
    
    if (!message || strlen(message) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Message is required");
    }
    
    if (!passphrase || strlen(passphrase) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Passphrase is required");
    }
    
    /* Build DecryptSymmetricRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_DecryptSymmetricRequest_start_as_root(&builder);
    
    /* Add message field */
    flatbuffers_string_ref_t message_ref = flatbuffers_string_create_str(&builder, message);
    model_DecryptSymmetricRequest_message_add(&builder, message_ref);
    
    /* Add passphrase field */
    flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(&builder, passphrase);
    model_DecryptSymmetricRequest_passphrase_add(&builder, passphrase_ref);
    
    /* Add options field if provided */
    model_KeyOptions_ref_t options_ref = build_key_options(&builder, options);
    if (options_ref) {
        model_DecryptSymmetricRequest_options_add(&builder, options_ref);
    }
    
    /* End the request */
    model_DecryptSymmetricRequest_end_as_root(&builder);
    
    /* Get buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, 
                                 "Failed to serialize DecryptSymmetricRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("decryptSymmetric", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse StringResponse */
    model_StringResponse_table_t string_response = model_StringResponse_as_root(response->message);
    if (!string_response) {
        /* Free response if needed */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to parse StringResponse");
    }
    
    /* Check for error in response */
    flatbuffers_string_t error = model_StringResponse_error(string_response);
    if (error && strlen(error) > 0) {
        char *error_copy = duplicate_string(error);
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_DECRYPTION_FAILED, error_copy);
    }
    
    /* Get the decrypted message */
    flatbuffers_string_t decrypted_str = model_StringResponse_output(string_response);
    if (!decrypted_str) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No decrypted message in response");
    }
    
    /* Duplicate the decrypted message string */
    char *decrypted_copy = duplicate_string(decrypted_str);
    if (!decrypted_copy) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to copy decrypted message");
    }
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(decrypted_copy, strlen(decrypted_copy) + 1);
}

openpgp_result_t openpgp_encrypt_symmetric_file(const char *input_file, const char *output_file,
                                               const char *passphrase,
                                               const openpgp_file_hints_t *file_hints,
                                               const openpgp_key_options_t *options) {
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }
    
    if (!input_file || strlen(input_file) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Input file path is required");
    }
    
    if (!output_file || strlen(output_file) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Output file path is required");
    }
    
    if (!passphrase || strlen(passphrase) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Passphrase is required");
    }
    
    /* Build EncryptSymmetricFileRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_EncryptSymmetricFileRequest_start_as_root(&builder);
    
    /* Add input field */
    flatbuffers_string_ref_t input_ref = flatbuffers_string_create_str(&builder, input_file);
    model_EncryptSymmetricFileRequest_input_add(&builder, input_ref);
    
    /* Add output field */
    flatbuffers_string_ref_t output_ref = flatbuffers_string_create_str(&builder, output_file);
    model_EncryptSymmetricFileRequest_output_add(&builder, output_ref);
    
    /* Add passphrase field */
    flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(&builder, passphrase);
    model_EncryptSymmetricFileRequest_passphrase_add(&builder, passphrase_ref);
    
    /* Add options field if provided */
    model_KeyOptions_ref_t options_ref = build_key_options(&builder, options);
    if (options_ref) {
        model_EncryptSymmetricFileRequest_options_add(&builder, options_ref);
    }
    
    /* Add file_hints field if provided */
    model_FileHints_ref_t file_hints_ref = build_file_hints(&builder, file_hints);
    if (file_hints_ref) {
        model_EncryptSymmetricFileRequest_file_hints_add(&builder, file_hints_ref);
    }
    
    /* End the request */
    model_EncryptSymmetricFileRequest_end_as_root(&builder);
    
    /* Get buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, 
                                 "Failed to serialize EncryptSymmetricFileRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("encryptSymmetricFile", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse StringResponse */
    model_StringResponse_table_t string_response = model_StringResponse_as_root(response->message);
    if (!string_response) {
        /* Free response if needed */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to parse StringResponse");
    }
    
    /* Check for error in response */
    flatbuffers_string_t error = model_StringResponse_error(string_response);
    if (error && strlen(error) > 0) {
        char *error_copy = duplicate_string(error);
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_ENCRYPTION_FAILED, error_copy);
    }
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(NULL, 0);
}

openpgp_result_t openpgp_decrypt_symmetric_file(const char *input_file, const char *output_file,
                                               const char *passphrase,
                                               const openpgp_key_options_t *options) {
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }
    
    if (!input_file || strlen(input_file) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Input file path is required");
    }
    
    if (!output_file || strlen(output_file) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Output file path is required");
    }
    
    if (!passphrase || strlen(passphrase) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Passphrase is required");
    }
    
    /* Build DecryptSymmetricFileRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_DecryptSymmetricFileRequest_start_as_root(&builder);
    
    /* Add input field */
    flatbuffers_string_ref_t input_ref = flatbuffers_string_create_str(&builder, input_file);
    model_DecryptSymmetricFileRequest_input_add(&builder, input_ref);
    
    /* Add output field */
    flatbuffers_string_ref_t output_ref = flatbuffers_string_create_str(&builder, output_file);
    model_DecryptSymmetricFileRequest_output_add(&builder, output_ref);
    
    /* Add passphrase field */
    flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(&builder, passphrase);
    model_DecryptSymmetricFileRequest_passphrase_add(&builder, passphrase_ref);
    
    /* Add options field if provided */
    model_KeyOptions_ref_t options_ref = build_key_options(&builder, options);
    if (options_ref) {
        model_DecryptSymmetricFileRequest_options_add(&builder, options_ref);
    }
    
    /* End the request */
    model_DecryptSymmetricFileRequest_end_as_root(&builder);
    
    /* Get buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, 
                                 "Failed to serialize DecryptSymmetricFileRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("decryptSymmetricFile", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse StringResponse */
    model_StringResponse_table_t string_response = model_StringResponse_as_root(response->message);
    if (!string_response) {
        /* Free response if needed */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to parse StringResponse");
    }
    
    /* Check for error in response */
    flatbuffers_string_t error = model_StringResponse_error(string_response);
    if (error && strlen(error) > 0) {
        char *error_copy = duplicate_string(error);
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_DECRYPTION_FAILED, error_copy);
    }
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(NULL, 0);
}

openpgp_result_t openpgp_encrypt_symmetric_bytes(const uint8_t *data, size_t data_len,
                                                const char *passphrase,
                                                const openpgp_file_hints_t *file_hints,
                                                const openpgp_key_options_t *options) {
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }
    
    if (!data || data_len == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Data is required");
    }
    
    if (!passphrase || strlen(passphrase) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Passphrase is required");
    }
    
    /* Build EncryptSymmetricBytesRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_EncryptSymmetricBytesRequest_start_as_root(&builder);
    
    /* Add message field as byte vector */
    flatbuffers_uint8_vec_ref_t message_ref = flatbuffers_uint8_vec_create(&builder, data, data_len);
    model_EncryptSymmetricBytesRequest_message_add(&builder, message_ref);
    
    /* Add passphrase field */
    flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(&builder, passphrase);
    model_EncryptSymmetricBytesRequest_passphrase_add(&builder, passphrase_ref);
    
    /* Add options field if provided */
    model_KeyOptions_ref_t options_ref = build_key_options(&builder, options);
    if (options_ref) {
        model_EncryptSymmetricBytesRequest_options_add(&builder, options_ref);
    }
    
    /* Add file_hints field if provided */
    model_FileHints_ref_t file_hints_ref = build_file_hints(&builder, file_hints);
    if (file_hints_ref) {
        model_EncryptSymmetricBytesRequest_file_hints_add(&builder, file_hints_ref);
    }
    
    /* End the request */
    model_EncryptSymmetricBytesRequest_end_as_root(&builder);
    
    /* Get buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, 
                                 "Failed to serialize EncryptSymmetricBytesRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("encryptSymmetricBytes", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse BytesResponse */
    model_BytesResponse_table_t bytes_response = model_BytesResponse_as_root(response->message);
    if (!bytes_response) {
        /* Free response if needed */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to parse BytesResponse");
    }
    
    /* Check for error in response */
    flatbuffers_string_t error = model_BytesResponse_error(bytes_response);
    if (error && strlen(error) > 0) {
        char *error_copy = duplicate_string(error);
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_ENCRYPTION_FAILED, error_copy);
    }
    
    /* Get the encrypted bytes */
    flatbuffers_uint8_vec_t encrypted_vec = model_BytesResponse_output(bytes_response);
    if (!encrypted_vec) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No encrypted data in response");
    }
    
    size_t encrypted_len = flatbuffers_uint8_vec_len(encrypted_vec);
    
    /* Duplicate the encrypted data */
    uint8_t *encrypted_copy = malloc(encrypted_len);
    if (!encrypted_copy) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to copy encrypted data");
    }
    
    /* Copy data element by element */
    for (size_t i = 0; i < encrypted_len; i++) {
        encrypted_copy[i] = flatbuffers_uint8_vec_at(encrypted_vec, i);
    }
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(encrypted_copy, encrypted_len);
}

openpgp_result_t openpgp_decrypt_symmetric_bytes(const uint8_t *data, size_t data_len,
                                                const char *passphrase,
                                                const openpgp_key_options_t *options) {
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }
    
    if (!data || data_len == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Data is required");
    }
    
    if (!passphrase || strlen(passphrase) == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Passphrase is required");
    }
    
    /* Build DecryptSymmetricBytesRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_DecryptSymmetricBytesRequest_start_as_root(&builder);
    
    /* Add message field as byte vector */
    flatbuffers_uint8_vec_ref_t message_ref = flatbuffers_uint8_vec_create(&builder, data, data_len);
    model_DecryptSymmetricBytesRequest_message_add(&builder, message_ref);
    
    /* Add passphrase field */
    flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(&builder, passphrase);
    model_DecryptSymmetricBytesRequest_passphrase_add(&builder, passphrase_ref);
    
    /* Add options field if provided */
    model_KeyOptions_ref_t options_ref = build_key_options(&builder, options);
    if (options_ref) {
        model_DecryptSymmetricBytesRequest_options_add(&builder, options_ref);
    }
    
    /* End the request */
    model_DecryptSymmetricBytesRequest_end_as_root(&builder);
    
    /* Get buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, 
                                 "Failed to serialize DecryptSymmetricBytesRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("decryptSymmetricBytes", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse BytesResponse */
    model_BytesResponse_table_t bytes_response = model_BytesResponse_as_root(response->message);
    if (!bytes_response) {
        /* Free response if needed */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to parse BytesResponse");
    }
    
    /* Check for error in response */
    flatbuffers_string_t error = model_BytesResponse_error(bytes_response);
    if (error && strlen(error) > 0) {
        char *error_copy = duplicate_string(error);
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_DECRYPTION_FAILED, error_copy);
    }
    
    /* Get the decrypted bytes */
    flatbuffers_uint8_vec_t decrypted_vec = model_BytesResponse_output(bytes_response);
    if (!decrypted_vec) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No decrypted data in response");
    }
    
    size_t decrypted_len = flatbuffers_uint8_vec_len(decrypted_vec);
    
    /* Duplicate the decrypted data */
    uint8_t *decrypted_copy = malloc(decrypted_len);
    if (!decrypted_copy) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to copy decrypted data");
    }
    
    /* Copy data element by element */
    for (size_t i = 0; i < decrypted_len; i++) {
        decrypted_copy[i] = flatbuffers_uint8_vec_at(decrypted_vec, i);
    }
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(decrypted_copy, decrypted_len);
}

/*
 * Asymmetric Encryption Operations
 */

openpgp_result_t openpgp_encrypt(const char *message, 
                                const char **recipient_keys,
                                size_t recipient_count,
                                const openpgp_key_options_t *options) {
    /* Input validation */
    if (!message) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Message cannot be null");
    }
    if (!recipient_keys) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Recipient keys cannot be null");
    }
    if (recipient_count == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Must have at least one recipient");
    }
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }

    /* For now, only support single recipient (FlatBuffer schema limitation) */
    if (recipient_count > 1) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, 
                                 "Multiple recipients not yet supported");
    }

    /* Build EncryptRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_EncryptRequest_start_as_root(&builder);
    
    /* Add message field */
    flatbuffers_string_ref_t message_ref = flatbuffers_string_create_str(&builder, message);
    model_EncryptRequest_message_add(&builder, message_ref);
    
    /* Add public_key field (only first recipient for now) */
    flatbuffers_string_ref_t public_key_ref = flatbuffers_string_create_str(&builder, recipient_keys[0]);
    model_EncryptRequest_public_key_add(&builder, public_key_ref);
    
    /* Add options field if provided */
    if (options) {
        model_KeyOptions_ref_t key_options_ref = build_key_options(&builder, options);
        model_EncryptRequest_options_add(&builder, key_options_ref);
    }
    
    model_EncryptRequest_end_as_root(&builder);
    
    /* Get the buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION,
                                 "Failed to serialize EncryptRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("encrypt", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse StringResponse */
    model_StringResponse_table_t string_response = model_StringResponse_as_root(response->message);
    if (!string_response) {
        /* Free response if needed */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to parse StringResponse");
    }
    
    /* Check for error in response */
    flatbuffers_string_t error = model_StringResponse_error(string_response);
    if (error && strlen(error) > 0) {
        char *error_copy = duplicate_string(error);
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_ENCRYPTION_FAILED, error_copy);
    }
    
    /* Get the encrypted message */
    flatbuffers_string_t encrypted_str = model_StringResponse_output(string_response);
    if (!encrypted_str) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No encrypted message in response");
    }
    
    /* Duplicate the encrypted message string */
    char *encrypted_copy = duplicate_string(encrypted_str);
    if (!encrypted_copy) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to copy encrypted message");
    }
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(encrypted_copy, strlen(encrypted_copy) + 1);
}

openpgp_result_t openpgp_decrypt(const char *message,
                                const char *private_key,
                                const char *passphrase,
                                const openpgp_key_options_t *options) {
    /* Input validation */
    if (!message) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Message cannot be null");
    }
    if (!private_key) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Private key cannot be null");
    }
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }

    /* Build DecryptRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_DecryptRequest_start_as_root(&builder);
    
    /* Add message field */
    flatbuffers_string_ref_t message_ref = flatbuffers_string_create_str(&builder, message);
    model_DecryptRequest_message_add(&builder, message_ref);
    
    /* Add private_key field */
    flatbuffers_string_ref_t private_key_ref = flatbuffers_string_create_str(&builder, private_key);
    model_DecryptRequest_private_key_add(&builder, private_key_ref);
    
    /* Add passphrase field if provided */
    if (passphrase) {
        flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(&builder, passphrase);
        model_DecryptRequest_passphrase_add(&builder, passphrase_ref);
    }
    
    /* Add options field if provided */
    if (options) {
        model_KeyOptions_ref_t key_options_ref = build_key_options(&builder, options);
        model_DecryptRequest_options_add(&builder, key_options_ref);
    }
    
    model_DecryptRequest_end_as_root(&builder);
    
    /* Get the buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION,
                                 "Failed to serialize DecryptRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("decrypt", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse StringResponse */
    model_StringResponse_table_t string_response = model_StringResponse_as_root(response->message);
    if (!string_response) {
        /* Free response if needed */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to parse StringResponse");
    }
    
    /* Check for error in response */
    flatbuffers_string_t error = model_StringResponse_error(string_response);
    if (error && strlen(error) > 0) {
        char *error_copy = duplicate_string(error);
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_DECRYPTION_FAILED, error_copy);
    }
    
    /* Get the decrypted message */
    flatbuffers_string_t decrypted_str = model_StringResponse_output(string_response);
    if (!decrypted_str) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No decrypted message in response");
    }
    
    /* Duplicate the decrypted message string */
    char *decrypted_copy = duplicate_string(decrypted_str);
    if (!decrypted_copy) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to copy decrypted message");
    }
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(decrypted_copy, strlen(decrypted_copy) + 1);
}

openpgp_result_t openpgp_encrypt_file(const char *input_file,
                                     const char *output_file,
                                     const char **recipient_keys,
                                     size_t recipient_count,
                                     const openpgp_file_hints_t *file_hints,
                                     const openpgp_key_options_t *options) {
    /* Input validation */
    if (!input_file || !output_file) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, 
                                 "Input and output file paths cannot be null");
    }
    if (!recipient_keys || recipient_count == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, 
                                 "Must have at least one recipient key");
    }
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }

    /* TODO: Implement file-based asymmetric encryption */
    return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, 
                             "Asymmetric file encryption not yet implemented");
}

openpgp_result_t openpgp_decrypt_file(const char *input_file,
                                     const char *output_file,
                                     const char *private_key,
                                     const char *passphrase,
                                     const openpgp_key_options_t *options) {
    /* Input validation */
    if (!input_file || !output_file) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, 
                                 "Input and output file paths cannot be null");
    }
    if (!private_key) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Private key cannot be null");
    }
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }

    /* TODO: Implement file-based asymmetric decryption */
    return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, 
                             "Asymmetric file decryption not yet implemented");
}

openpgp_result_t openpgp_encrypt_bytes(const uint8_t *data, size_t data_len,
                                      const char **recipient_keys,
                                      size_t recipient_count,
                                      const openpgp_file_hints_t *file_hints,
                                      const openpgp_key_options_t *options) {
    /* Input validation */
    if (!data || data_len == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, 
                                 "Data cannot be null and length must be > 0");
    }
    if (!recipient_keys || recipient_count == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, 
                                 "Must have at least one recipient key");
    }
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }

    /* TODO: Implement binary asymmetric encryption */
    return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, 
                             "Asymmetric binary encryption not yet implemented");
}

openpgp_result_t openpgp_decrypt_bytes(const uint8_t *data, size_t data_len,
                                      const char *private_key,
                                      const char *passphrase,
                                      const openpgp_key_options_t *options) {
    /* Input validation */
    if (!data || data_len == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, 
                                 "Data cannot be null and length must be > 0");
    }
    if (!private_key) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Private key cannot be null");
    }
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }

    /* TODO: Implement binary asymmetric decryption */
    return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, 
                             "Asymmetric binary decryption not yet implemented");
}

/*
 * Signing Operations
 */

openpgp_result_t openpgp_sign(const char *message,
                             const char *private_key,
                             const char *passphrase,
                             const openpgp_key_options_t *options) {
    /* Input validation */
    if (!message) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Message cannot be null");
    }
    if (!private_key) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Private key cannot be null");
    }
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }

    /* Build SignRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_SignRequest_start_as_root(&builder);
    
    /* Add message field */
    flatbuffers_string_ref_t message_ref = flatbuffers_string_create_str(&builder, message);
    model_SignRequest_message_add(&builder, message_ref);
    
    /* Add private_key field */
    flatbuffers_string_ref_t private_key_ref = flatbuffers_string_create_str(&builder, private_key);
    model_SignRequest_private_key_add(&builder, private_key_ref);
    
    /* Add passphrase field if provided */
    if (passphrase) {
        flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(&builder, passphrase);
        model_SignRequest_passphrase_add(&builder, passphrase_ref);
    }
    
    /* Add options field if provided */
    if (options) {
        model_KeyOptions_ref_t key_options_ref = build_key_options(&builder, options);
        model_SignRequest_options_add(&builder, key_options_ref);
    }
    
    model_SignRequest_end_as_root(&builder);
    
    /* Get the buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION,
                                 "Failed to serialize SignRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("sign", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse StringResponse */
    model_StringResponse_table_t string_response = model_StringResponse_as_root(response->message);
    if (!string_response) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Invalid response format");
    }
    
    /* Check for errors */
    flatbuffers_string_t error_str = model_StringResponse_error(string_response);
    if (error_str && strlen(error_str) > 0) {
        char *error_copy = duplicate_string(error_str);
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SIGNING_FAILED, error_copy);
    }
    
    /* Get the signature */
    flatbuffers_string_t signature_str = model_StringResponse_output(string_response);
    if (!signature_str) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No signature in response");
    }
    
    /* Duplicate the signature string */
    char *signature_copy = duplicate_string(signature_str);
    if (!signature_copy) {
        /* Free response */
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to copy signature");
    }
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(signature_copy, strlen(signature_copy) + 1);
}

openpgp_result_t openpgp_sign_data(const char *message,
                                  const char *private_key,
                                  const char *passphrase,
                                  const openpgp_key_options_t *options) {
    /* Input validation */
    if (!message) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Message cannot be null");
    }
    if (!private_key) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Private key cannot be null");
    }
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }

    /* Build SignDataRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_SignDataRequest_start_as_root(&builder);
    
    /* Add message field */
    flatbuffers_string_ref_t message_ref = flatbuffers_string_create_str(&builder, message);
    model_SignDataRequest_message_add(&builder, message_ref);
    
    /* Add private_key field */
    flatbuffers_string_ref_t private_key_ref = flatbuffers_string_create_str(&builder, private_key);
    model_SignDataRequest_private_key_add(&builder, private_key_ref);
    
    /* Add passphrase field if provided */
    if (passphrase) {
        flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(&builder, passphrase);
        model_SignDataRequest_passphrase_add(&builder, passphrase_ref);
    }
    
    /* Add options field if provided */
    if (options) {
        model_KeyOptions_ref_t key_options_ref = build_key_options(&builder, options);
        model_SignDataRequest_options_add(&builder, key_options_ref);
    }
    
    model_SignDataRequest_end_as_root(&builder);
    
    /* Get the buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION,
                                 "Failed to serialize SignDataRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("signData", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response - same as openpgp_sign */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse StringResponse */
    model_StringResponse_table_t string_response = model_StringResponse_as_root(response->message);
    if (!string_response) {
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Invalid response format");
    }
    
    /* Check for errors */
    flatbuffers_string_t error_str = model_StringResponse_error(string_response);
    if (error_str && strlen(error_str) > 0) {
        char *error_copy = duplicate_string(error_str);
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SIGNING_FAILED, error_copy);
    }
    
    /* Get the signature */
    flatbuffers_string_t signature_str = model_StringResponse_output(string_response);
    if (!signature_str) {
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No signature in response");
    }
    
    /* Duplicate the signature string */
    char *signature_copy = duplicate_string(signature_str);
    if (!signature_copy) {
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to copy signature");
    }
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(signature_copy, strlen(signature_copy) + 1);
}

openpgp_result_t openpgp_sign_file(const char *input_file,
                                  const char *private_key,
                                  const char *passphrase,
                                  const openpgp_key_options_t *options) {
    /* TODO: Implement file signing */
    return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, 
                             "File signing not yet implemented");
}

openpgp_result_t openpgp_sign_bytes(const uint8_t *data, size_t data_len,
                                   const char *private_key,
                                   const char *passphrase,
                                   const openpgp_key_options_t *options) {
    /* Input validation */
    if (!data || data_len == 0) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Data cannot be null and length must be > 0");
    }
    if (!private_key) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, "Private key cannot be null");
    }
    if (!g_openpgp.initialized) {
        return create_error_result(OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED,
                                 "Library not initialized");
    }

    /* Build SignBytesRequest */
    flatcc_builder_t builder;
    flatcc_builder_init(&builder);
    
    /* Create the request manually */
    model_SignBytesRequest_start_as_root(&builder);
    
    /* Add message field (as byte vector) */
    flatbuffers_uint8_vec_ref_t message_ref = flatbuffers_uint8_vec_create(&builder, data, data_len);
    model_SignBytesRequest_message_add(&builder, message_ref);
    
    /* Add private_key field */
    flatbuffers_string_ref_t private_key_ref = flatbuffers_string_create_str(&builder, private_key);
    model_SignBytesRequest_private_key_add(&builder, private_key_ref);
    
    /* Add passphrase field if provided */
    if (passphrase) {
        flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(&builder, passphrase);
        model_SignBytesRequest_passphrase_add(&builder, passphrase_ref);
    }
    
    /* Add options field if provided */
    if (options) {
        model_KeyOptions_ref_t key_options_ref = build_key_options(&builder, options);
        model_SignBytesRequest_options_add(&builder, key_options_ref);
    }
    
    model_SignBytesRequest_end_as_root(&builder);
    
    /* Get the buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(&builder, &size);
    if (!buffer) {
        flatcc_builder_clear(&builder);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION,
                                 "Failed to serialize SignBytesRequest");
    }
    
    /* Call bridge */
    BytesReturn* response = g_openpgp.bridge_call("signBytes", buffer, size);
    
    /* Free the builder and buffer */
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(&builder);
    
    /* Handle response - same pattern as other signing functions */
    if (!response) {
        return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, "Bridge call failed");
    }
    
    /* Parse StringResponse */
    model_StringResponse_table_t string_response = model_StringResponse_as_root(response->message);
    if (!string_response) {
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Invalid response format");
    }
    
    /* Check for errors */
    flatbuffers_string_t error_str = model_StringResponse_error(string_response);
    if (error_str && strlen(error_str) > 0) {
        char *error_copy = duplicate_string(error_str);
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SIGNING_FAILED, error_copy);
    }
    
    /* Get the signature */
    flatbuffers_string_t signature_str = model_StringResponse_output(string_response);
    if (!signature_str) {
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "No signature in response");
    }
    
    /* Duplicate the signature string */
    char *signature_copy = duplicate_string(signature_str);
    if (!signature_copy) {
        if (response->error) free(response->error);
        if (response->message) free(response->message);
        free(response);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to copy signature");
    }
    
    /* Free response */
    if (response->error) free(response->error);
    if (response->message) free(response->message);
    free(response);
    
    return create_success_result(signature_copy, strlen(signature_copy) + 1);
}

openpgp_result_t openpgp_sign_data_bytes(const uint8_t *data, size_t data_len,
                                        const char *private_key,
                                        const char *passphrase,
                                        const openpgp_key_options_t *options) {
    /* TODO: Implement data bytes signing */
    return create_error_result(OPENPGP_ERROR_BRIDGE_CALL, 
                             "Data bytes signing not yet implemented");
}