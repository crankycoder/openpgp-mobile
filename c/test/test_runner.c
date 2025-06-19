#include "test_framework.h"

/* Global test counters */
int g_tests_run = 0;        /* Total assertions/subtests */
int g_tests_failed = 0;     /* Failed assertions/subtests */
int g_major_tests_run = 0;  /* Major test functions */
int g_major_tests_failed = 0; /* Failed major test functions */

/* Test function declarations */
extern int test_basic_initialization(void);
extern int test_error_handling(void);
extern int test_memory_management(void);
extern int test_helper_functions(void);
extern int test_bridge_integration(void);
extern int test_generate_key_basic(void);
extern int test_generate_key_with_options(void);
extern int test_generate_key_input_validation(void);
extern int test_generate_key_without_init(void);
extern int test_create_generate_request(void);
extern int test_parse_keypair_response(void);
extern int test_flatbuffer_serialization_simple(void);
extern int test_flatbuffer_serialization_with_strings(void);
extern int test_flatbuffer_serialization_full_request(void);
extern int test_build_empty_generate_request(void);
extern int test_build_request_with_empty_options(void);
extern int test_build_request_with_one_string(void);

/* Symmetric encryption test declarations */
extern int test_symmetric_encrypt_decrypt_message(void);
extern int test_symmetric_encrypt_decrypt_with_file_hints(void);
extern int test_symmetric_encrypt_decrypt_with_options(void);
extern int test_symmetric_encrypt_decrypt_bytes(void);
extern int test_symmetric_encrypt_decrypt_file(void);
extern int test_symmetric_error_handling(void);
extern int test_symmetric_uninitialized_library(void);

/* Key operation test declarations */
extern void run_convert_tests(void);
extern void run_metadata_tests(void);

/* Asymmetric encryption test declarations */
extern int test_encrypt_message_single_recipient(void);
extern int test_encrypt_message_multiple_recipients(void);
extern int test_decrypt_message(void);
extern int test_encrypt_decrypt_round_trip(void);
extern int test_encrypt_invalid_public_key(void);
extern int test_encrypt_null_message(void);
extern int test_encrypt_null_recipients(void);
extern int test_encrypt_zero_recipients(void);
extern int test_decrypt_wrong_private_key(void);

int main(void) {
    printf(COLOR_BLUE "OpenPGP C Wrapper Library Tests" COLOR_RESET "\n");
    printf("======================================\n\n");

    /* Reset counters */
    g_tests_run = 0;
    g_tests_failed = 0;
    g_major_tests_run = 0;
    g_major_tests_failed = 0;

    /* Run incremental builder tests first */
    printf("\n" COLOR_BLUE "Incremental Builder Tests" COLOR_RESET "\n");
    printf("-------------------------\n");
    
    RUN_TEST(build_empty_generate_request);
    RUN_TEST(build_request_with_empty_options);
    RUN_TEST(build_request_with_one_string);
    
    /* Run FlatBuffer serialization tests */
    printf("\n" COLOR_BLUE "FlatBuffer Serialization Tests" COLOR_RESET "\n");
    printf("------------------------------\n");
    
    RUN_TEST(flatbuffer_serialization_simple);
    RUN_TEST(flatbuffer_serialization_with_strings);
    RUN_TEST(flatbuffer_serialization_full_request);
    
    /* Run infrastructure tests */
    printf("\n" COLOR_BLUE "Infrastructure Tests" COLOR_RESET "\n");
    printf("--------------------\n");
    
    RUN_TEST(basic_initialization);
    RUN_TEST(error_handling);
    RUN_TEST(memory_management);
    RUN_TEST(helper_functions);
    RUN_TEST(bridge_integration);
    
    printf("\n" COLOR_BLUE "Key Generation Tests" COLOR_RESET "\n");
    printf("--------------------\n");
    
    /* Run key generation tests */
    RUN_TEST(generate_key_basic);
    RUN_TEST(generate_key_with_options);
    RUN_TEST(generate_key_input_validation);
    RUN_TEST(generate_key_without_init);
    
    printf("\n" COLOR_BLUE "FlatBuffer Tests" COLOR_RESET "\n");
    printf("----------------\n");
    
    /* Run FlatBuffer tests */
    RUN_TEST(create_generate_request);
    RUN_TEST(parse_keypair_response);

    printf("\n" COLOR_BLUE "Symmetric Encryption Tests" COLOR_RESET "\n");
    printf("--------------------------\n");
    
    /* Run symmetric encryption tests */
    RUN_TEST(symmetric_uninitialized_library);
    RUN_TEST(symmetric_error_handling);
    RUN_TEST(symmetric_encrypt_decrypt_message);
    RUN_TEST(symmetric_encrypt_decrypt_with_file_hints);
    RUN_TEST(symmetric_encrypt_decrypt_with_options);
    RUN_TEST(symmetric_encrypt_decrypt_bytes);
    RUN_TEST(symmetric_encrypt_decrypt_file);

    /* Run key operation tests */
    run_convert_tests();
    run_metadata_tests();

    printf("\n" COLOR_BLUE "Asymmetric Encryption Tests" COLOR_RESET "\n");
    printf("----------------------------\n");
    
    /* Run asymmetric encryption tests */
    RUN_TEST(encrypt_message_single_recipient);
    RUN_TEST(encrypt_message_multiple_recipients);
    RUN_TEST(decrypt_message);
    RUN_TEST(encrypt_decrypt_round_trip);
    RUN_TEST(encrypt_invalid_public_key);
    RUN_TEST(encrypt_null_message);
    RUN_TEST(encrypt_null_recipients);
    RUN_TEST(encrypt_zero_recipients);
    RUN_TEST(decrypt_wrong_private_key);

    /* Print summary */
    printf("\n======================================\n");
    printf("Major tests run: %d\n", g_major_tests_run);
    printf("Major tests failed: %d\n", g_major_tests_failed);
    printf("Total subtests run: %d\n", g_tests_run);
    printf("Total subtests failed: %d\n", g_tests_failed);
    
    if (g_tests_failed == 0) {
        printf(COLOR_GREEN "All tests passed!" COLOR_RESET "\n");
        return 0;
    } else {
        printf(COLOR_RED "%d major tests failed, %d subtests failed!" COLOR_RESET "\n", 
               g_major_tests_failed, g_tests_failed);
        return 1;
    }
}