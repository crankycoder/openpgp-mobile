/* 
 * FlatCC Runtime Stubs for Phase 1
 * 
 * These are stub implementations to allow compilation and testing
 * of the C wrapper infrastructure. In Phase 1, we expect operations
 * to fail gracefully since the actual bridge is not yet functional.
 * 
 * These stubs will be replaced with proper flatcc runtime in later phases.
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Forward declare types without including headers */
struct flatcc_builder;
typedef struct flatcc_builder flatcc_builder_t;
typedef uint64_t flatcc_builder_ref_t;
typedef int flatcc_builder_vt_t;

/* Builder stubs that return errors to test defensive code paths */
int flatcc_builder_init(flatcc_builder_t *B) {
    (void)B;
    return -1; /* Return failure to test error handling */
}

void flatcc_builder_clear(flatcc_builder_t *B) {
    (void)B;
}

size_t flatcc_builder_get_buffer_size(flatcc_builder_t *B) {
    (void)B;
    return 0;
}

void *flatcc_builder_get_direct_buffer(flatcc_builder_t *B, size_t *size) {
    (void)B;
    if (size) *size = 0;
    return NULL;
}

void *flatcc_builder_finalize_buffer(flatcc_builder_t *B, size_t *size) {
    (void)B;
    if (size) *size = 0;
    return NULL;
}

int flatcc_builder_reset(flatcc_builder_t *B) {
    (void)B;
    return 0;
}

flatcc_builder_ref_t flatcc_builder_end_buffer(flatcc_builder_t *B, const char *identifier) {
    (void)B;
    (void)identifier;
    return 0;
}/* Table operations stubs */
int flatcc_builder_start_table(flatcc_builder_t *B, int count) {
    (void)B;
    (void)count;
    return 0;
}

flatcc_builder_ref_t flatcc_builder_end_table(flatcc_builder_t *B) {
    (void)B;
    return 0;
}

int flatcc_builder_check_required(flatcc_builder_t *B, const flatcc_builder_vt_t *ids, int len) {
    (void)B;
    (void)ids;
    (void)len;
    return 0;
}

/* Table field stubs */
flatcc_builder_ref_t flatcc_builder_table_add(flatcc_builder_t *B, int id, const void *data, size_t size, uint16_t align) {
    (void)B;
    (void)id;
    (void)data;
    (void)size;
    (void)align;
    return 0;
}

flatcc_builder_ref_t flatcc_builder_table_add_copy(flatcc_builder_t *B, int id, const void *data, size_t size, uint16_t align) {
    (void)B;
    (void)id;
    (void)data;
    (void)size;
    (void)align;
    return 0;
}

flatcc_builder_ref_t flatcc_builder_table_add_offset(flatcc_builder_t *B, int id, flatcc_builder_ref_t ref) {
    (void)B;
    (void)id;
    (void)ref;
    return 0;
}/* String operations stubs */
flatcc_builder_ref_t flatcc_builder_create_string(flatcc_builder_t *B, const char *s, size_t len) {
    (void)B;
    (void)s;
    (void)len;
    return 0;
}

flatcc_builder_ref_t flatcc_builder_create_string_str(flatcc_builder_t *B, const char *s) {
    (void)B;
    (void)s;
    return 0;
}

/* Vector operations stubs */
flatcc_builder_ref_t flatcc_builder_create_vector(flatcc_builder_t *B, const void *data, size_t count, size_t elem_size, uint16_t align, size_t max_count) {
    (void)B;
    (void)data;
    (void)count;
    (void)elem_size;
    (void)align;
    (void)max_count;
    return 0;
}

flatcc_builder_ref_t flatcc_builder_start_offset_vector(flatcc_builder_t *B) {
    (void)B;
    return 0;
}

flatcc_builder_ref_t flatcc_builder_offset_vector_push(flatcc_builder_t *B, flatcc_builder_ref_t ref) {
    (void)B;
    (void)ref;
    return 0;
}

flatcc_builder_ref_t flatcc_builder_end_offset_vector(flatcc_builder_t *B) {
    (void)B;
    return 0;
}/* Refmap stubs */
void *flatcc_refmap_find(void *map, const void *key) {
    (void)map;
    (void)key;
    return NULL;
}

int flatcc_refmap_insert(void *map, const void *key, void *value) {
    (void)map;
    (void)key;
    (void)value;
    return -1;
}

