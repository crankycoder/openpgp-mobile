#include <stdio.h>
#include "flatcc/flatcc_builder.h"

int main() {
    printf("Testing libflatccrt.a...\n");
    
    flatcc_builder_t builder;
    
    printf("Calling flatcc_builder_init...\n");
    int result = flatcc_builder_init(&builder);
    printf("Result: %d\n", result);
    
    if (result == 0) {
        printf("Success! Builder initialized.\n");
        
        printf("Getting buffer size...\n");
        size_t size = flatcc_builder_get_buffer_size(&builder);
        printf("Buffer size: %zu\n", size);
        
        printf("Clearing builder...\n");
        flatcc_builder_clear(&builder);
        printf("Builder cleared.\n");
    } else {
        printf("Failed to initialize builder!\n");
    }
    
    return 0;
}