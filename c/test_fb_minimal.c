#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include FlatCC headers in correct order */
#include "flatcc/flatcc.h"
#include "bridge_builder.h"
#include "bridge_reader.h"

int main() {
    printf("Minimal FlatBuffer test\n");
    
    /* Initialize builder */
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    flatcc_builder_init(B);
    
    printf("Creating empty Options...\n");
    
    /* Start building */
    model_Options_start(B);
    model_Options_ref_t opts = model_Options_end(B);
    
    printf("Creating GenerateRequest...\n");
    
    model_GenerateRequest_start(B);
    model_GenerateRequest_options_add(B, opts);
    model_GenerateRequest_ref_t req = model_GenerateRequest_end(B);
    
    printf("Finishing buffer...\n");
    
    flatcc_builder_end_buffer(B, req);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    
    printf("Buffer size: %zu\n", size);
    
    flatcc_builder_clear(B);
    
    return 0;
}