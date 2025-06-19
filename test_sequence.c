int main() {
    // Run just the key generation integration tests
    extern int test_rsa_2048_generation_and_usage(void);
    extern int test_rsa_2048_with_passphrase_generation_and_usage(void);
    extern int test_ecdsa_p256_generation_and_usage(void);
    extern int test_ed25519_generation_and_usage(void);
    
    printf("Running key generation tests...
");
    test_rsa_2048_generation_and_usage();
    test_rsa_2048_with_passphrase_generation_and_usage();
    test_ecdsa_p256_generation_and_usage();
    test_ed25519_generation_and_usage();
    
    printf("Now testing sign_data with new generated key...
");
    
    // Initialize library
    openpgp_init();
    
    // Generate simple key
    openpgp_result_t gen_result = openpgp_generate_key("Test User", "test@example.com", NULL);
    openpgp_keypair_t* keypair = (openpgp_keypair_t*)gen_result.data;
    
    // Test sign_data
    printf("Calling openpgp_sign_data...
");
    openpgp_result_t result = openpgp_sign_data("test", keypair->private_key, NULL, NULL);
    printf("Result: %d
", result.error);
    
    return 0;
}
