#include <stdio.h>
#include <string.h>
#include "bridge_builder.h"
#include "bridge_reader.h"

int main() {
    printf("Creating complete GenerateRequest...\n");
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    if (flatcc_builder_init(B)) {
        printf("Failed to init builder\n");
        return 1;
    }
    
    /* Create strings */
    flatbuffers_string_ref_t name_ref = flatbuffers_string_create_str(B, "Test User");
    flatbuffers_string_ref_t email_ref = flatbuffers_string_create_str(B, "test@example.com");
    flatbuffers_string_ref_t passphrase_ref = flatbuffers_string_create_str(B, "testpass");
    
    /* Create KeyOptions */
    model_KeyOptions_ref_t key_opts = model_KeyOptions_create(B,
        0,      /* algorithm - RSA */
        2,      /* curve - P256 */
        0,      /* hash - SHA256 */
        0,      /* cipher - AES128 */
        0,      /* compression - NONE */
        -1,     /* compression_level */
        2048    /* rsa_bits */
    );
    
    /* Create Options */
    model_Options_ref_t opts = model_Options_create(B,
        name_ref,       /* name */
        0,              /* comment */
        email_ref,      /* email */
        passphrase_ref, /* passphrase */
        key_opts        /* key_options */
    );
    
    /* Create GenerateRequest as root */
    model_GenerateRequest_create_as_root(B, opts);
    
    /* Finalize the buffer */
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(B, &size);
    
    if (!buffer) {
        printf("Failed to finalize buffer\n");
        flatcc_builder_clear(B);
        return 1;
    }
    
    printf("Buffer created, size: %zu\n", size);
    
    /* Print full hex dump */
    unsigned char *bytes = (unsigned char *)buffer;
    printf("Full hex dump:\n");
    for (size_t i = 0; i < size; i++) {
        printf("%02x ", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    if (size % 16 != 0) printf("\n");
    
    /* Detailed analysis */
    printf("\nAnalyzing buffer structure:\n");
    printf("Offset 0-3 (root offset): %02x %02x %02x %02x\n", 
           bytes[0], bytes[1], bytes[2], bytes[3]);
    
    uint32_t root_offset = *(uint32_t*)bytes;
    printf("Root offset (little-endian): %u\n", root_offset);
    
    if (root_offset + 4 <= size) {
        printf("Root table starts at: %u\n", root_offset);
        printf("First 16 bytes of root table: ");
        for (size_t i = root_offset; i < root_offset + 16 && i < size; i++) {
            printf("%02x ", bytes[i]);
        }
        printf("\n");
    }
    
    /* Try to parse it back */
    printf("\nParsing buffer...\n");
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    if (parsed) {
        printf("GenerateRequest parse successful\n");
        
        model_Options_table_t parsed_opts = model_GenerateRequest_options(parsed);
        if (parsed_opts) {
            printf("Options parse successful\n");
            
            const char *name = model_Options_name(parsed_opts);
            const char *email = model_Options_email(parsed_opts);
            const char *pass = model_Options_passphrase(parsed_opts);
            
            printf("Name: %s\n", name ? name : "(null)");
            printf("Email: %s\n", email ? email : "(null)");
            printf("Passphrase: %s\n", pass ? pass : "(null)");
            
            model_KeyOptions_table_t ko = model_Options_key_options(parsed_opts);
            if (ko) {
                printf("KeyOptions: algorithm=%d, rsa_bits=%d\n",
                       model_KeyOptions_algorithm(ko),
                       model_KeyOptions_rsa_bits(ko));
            }
        }
    } else {
        printf("Parse failed\n");
    }
    
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(B);
    
    return 0;
}