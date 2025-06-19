#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "bridge_builder.h"
#include "bridge_reader.h"

typedef struct {
    void* message;
    int size;
    char* error;
} BytesReturn;

typedef BytesReturn* (*OpenPGPBridgeCall_fn)(char* name, void* payload, int payloadSize);

int main() {
    /* Load the bridge library */
    void *handle = dlopen("../output/binding/libopenpgp_bridge.so", RTLD_LAZY);
    if (!handle) {
        printf("Failed to load bridge: %s\n", dlerror());
        return 1;
    }
    
    OpenPGPBridgeCall_fn bridge_call = dlsym(handle, "OpenPGPBridgeCall");
    if (!bridge_call) {
        printf("Failed to find OpenPGPBridgeCall: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }
    
    /* Create a simple FlatBuffer */
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    if (flatcc_builder_init(B)) {
        printf("Failed to init builder\n");
        return 1;
    }
    
    /* Create strings */
    flatbuffers_string_ref_t email_ref = flatbuffers_string_create_str(B, "test@example.com");
    flatbuffers_string_ref_t name_ref = flatbuffers_string_create_str(B, "Test User");
    
    /* Create KeyOptions */
    model_KeyOptions_ref_t key_opts = model_KeyOptions_create(B,
        0,    /* algorithm - RSA */
        2,    /* curve - P256 */
        0,    /* hash - SHA256 */
        0,    /* cipher - AES128 */
        0,    /* compression - NONE */
        -1,   /* compression_level */
        2048  /* rsa_bits */
    );
    
    /* Create Options */
    model_Options_ref_t opts = model_Options_create(B,
        name_ref,       /* name */
        0,              /* comment */
        email_ref,      /* email */
        0,              /* passphrase */
        key_opts        /* key_options */
    );
    
    /* Create GenerateRequest as root */
    model_GenerateRequest_create_as_root(B, opts);
    
    /* Get buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(B, &size);
    
    printf("Created FlatBuffer, size: %zu\n", size);
    if (size >= 8) {
        unsigned char *bytes = (unsigned char *)buffer;
        printf("First 8 bytes: %02x %02x %02x %02x %02x %02x %02x %02x\n",
               bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7]);
    }
    
    /* Try to parse it locally first */
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    if (parsed) {
        printf("Local parse successful\n");
        model_Options_table_t parsed_opts = model_GenerateRequest_options(parsed);
        if (parsed_opts) {
            printf("Got options\n");
            const char *email = model_Options_email(parsed_opts);
            if (email) {
                printf("Email: %s\n", email);
            }
        }
    } else {
        printf("Local parse failed\n");
    }
    
    /* Call bridge */
    printf("\nCalling bridge...\n");
    BytesReturn *result = bridge_call("generate", buffer, (int)size);
    
    if (result) {
        if (result->error) {
            printf("Bridge error: %s\n", result->error);
        } else {
            printf("Bridge returned size: %d\n", result->size);
        }
    } else {
        printf("Bridge returned NULL\n");
    }
    
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(B);
    dlclose(handle);
    
    return 0;
}