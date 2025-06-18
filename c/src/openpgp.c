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
    model_KeyOptions_ref_t key_options_ref = model_KeyOptions_create(B,
        (model_Algorithm_enum_t)options->key_options.algorithm,   /* algorithm */
        (model_Curve_enum_t)options->key_options.curve,          /* curve */
        (model_Hash_enum_t)options->key_options.hash,            /* hash */
        (model_Cipher_enum_t)options->key_options.cipher,        /* cipher */
        (model_Compression_enum_t)options->key_options.compression, /* compression */
        options->key_options.compression_level,                   /* compression_level */
        options->key_options.rsa_bits                            /* rsa_bits */
    );
    
    /* Create Options */
    model_Options_ref_t options_ref = model_Options_create(B,
        name_ref,       /* name */
        comment_ref,    /* comment */  
        email_ref,      /* email */
        passphrase_ref, /* passphrase */
        key_options_ref /* key_options */
    );
    
    /* Create GenerateRequest */
    model_GenerateRequest_ref_t request_ref = model_GenerateRequest_create(B, options_ref);
    
    /* Finish buffer */
    if (!flatcc_builder_end_buffer(B, request_ref)) {
        flatcc_builder_clear(B);
        return create_error_result(OPENPGP_ERROR_SERIALIZATION, "Failed to create FlatBuffer");
    }
    
    /* Get buffer data */
    *buffer_size = flatcc_builder_get_buffer_size(B);
    void *data = malloc(*buffer_size);
    if (!data) {
        flatcc_builder_clear(B);
        return create_error_result(OPENPGP_ERROR_MEMORY_ALLOCATION, "Failed to allocate buffer");
    }
    
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