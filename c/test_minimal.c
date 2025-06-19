#include <stdio.h>
#include "bridge_builder.h"
#include "bridge_reader.h"

int main() {
    printf("Starting minimal FlatBuffer test...\n");
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    printf("Initializing builder...\n");
    if (flatcc_builder_init(B)) {
        printf("Failed to init builder\n");
        return 1;
    }
    
    printf("Creating empty Options...\n");
    model_Options_ref_t opts = model_Options_create(B, 0, 0, 0, 0, 0);
    
    printf("Creating GenerateRequest as root...\n");
    model_GenerateRequest_create_as_root(B, opts);
    
    printf("Finalizing buffer...\n");
    size_t size;
    void *buffer = flatcc_builder_finalize_aligned_buffer(B, &size);
    
    if (!buffer) {
        printf("Failed to finalize buffer\n");
        flatcc_builder_clear(B);
        return 1;
    }
    
    printf("Buffer created, size: %zu\n", size);
    
    /* Print hex dump */
    unsigned char *bytes = (unsigned char *)buffer;
    printf("Hex dump:\n");
    for (size_t i = 0; i < size; i++) {
        printf("%02x ", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
    
    /* Try to parse it back */
    printf("Parsing buffer...\n");
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    if (parsed) {
        printf("Parse successful\n");
    } else {
        printf("Parse failed\n");
    }
    
    flatcc_builder_aligned_free(buffer);
    flatcc_builder_clear(B);
    
    return 0;
}