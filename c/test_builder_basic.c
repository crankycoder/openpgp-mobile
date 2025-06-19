#include <stdio.h>
#include <assert.h>
#include "flatcc/flatcc_builder.h"
#include "bridge_builder.h"

int main() {
    printf("Testing basic builder operations...\n");
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    assert(flatcc_builder_init(B) == 0);
    printf("Builder initialized\n");
    
    // Try to start building a GenerateRequest
    printf("Starting GenerateRequest...\n");
    int ret = model_GenerateRequest_start(B);
    printf("Start result: %d\n", ret);
    
    if (ret == 0) {
        printf("Ending GenerateRequest...\n");
        model_GenerateRequest_ref_t ref = model_GenerateRequest_end(B);
        printf("Ref value: %ld\n", (long)ref);
        
        if (ref != 0) {
            printf("Finishing buffer...\n");
            flatbuffers_buffer_ref_t buf_ref = flatcc_builder_end_buffer(B, ref);
            printf("Buffer ref: %ld\n", (long)buf_ref);
            
            size_t size = flatcc_builder_get_buffer_size(B);
            printf("Buffer size: %zu\n", size);
        }
    }
    
    flatcc_builder_clear(B);
    printf("Done\n");
    
    return 0;
}