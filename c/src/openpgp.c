#include "openpgp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

/* TODO: FlatBuffers includes will be added once flatcc is available
#include "bridge_builder.h"
#include "bridge_reader.h"
*/

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

    /* TODO: Implement FlatBuffer serialization and bridge call */
    return create_error_result(OPENPGP_ERROR_UNKNOWN, "Key generation not yet implemented");
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