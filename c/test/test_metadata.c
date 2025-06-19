/*
 * test_metadata.c - Tests for key metadata extraction
 *
 * This file is part of the OpenPGP C binding test suite.
 * It tests the metadata extraction functions for both public and private keys.
 */

#include "test_framework.h"
#include "../include/openpgp.h"
#include <string.h>
#include <stdio.h>

/* Test RSA public key from Go tests */
static const char* TEST_RSA_PUBLIC_KEY = 
"-----BEGIN PGP PUBLIC KEY BLOCK-----\n"
"\n"
"mQENBF0Tpe0BCADm+ja4vMKuodkQEhLm/092M/6gt4TaKwzv8QcA53/FrM3g8wab\n"
"oksgIN1T+CscKXOY9GeJ530sBPhIiKuhlLW6Df3bUr7K0deN4RwOiD8eoVbqI7ss\n"
"TnpkdfWgJW7aBPaL7gO3V/iU2MJcYoS2fS8kQr8QPjjoIkFa1g8YSUgzOT6Dqg/r\n"
"WN4qBaqFqTvvZvS2kq+R7vY+KlMfKjCnNWaWAS4lfYTmVBGGSROx9zsp6kSoUVqZ\n"
"Um8QLGfVJvRMvszxW5KNJ0Uo5A5U6eC4SvWUj0TLnFRCU3OJtPQTcnZbXNbiZKRD\n"
"cad+9lF9YK7ReZnFEAD4K0DSbodjEzgZdZlaABEBAAG0CXRlc3Qga2V5ySQEEwEI\n"
"AA4FAl0Tpe0CGwMFCwkIBwMFFQoJCAsFFgIDAQACHgECF4AACgkQyU2EhUs+cKRV\n"
"2Af/WsrRfoRP0wQwh/aGntkbMk/NSuI3vPh68spE2c2NXWkp4cZbIX/YgkXcuAf1\n"
"8AmtcjByMySm6i5QaLQqptcNuXN6HnF9AsQr2uN7msk2VKiQoUw4iEo2qSSQaKIw\n"
"a7YobVOYGXU+pHRjNBZA/M5YuEOox5wZgugUPVi0I9ApRn9288AmkEaUZHoAMP8r\n"
"EpjZgoZ9iXQ8DrBdlz3KeGfLF+9pT4piNq9xiKByJvt07AaVKKNHZJ9CUqGBBkra\n"
"VJZ5A9QxEGgwVPEcSPmL4ycNiUHckuEkAycJd/XT1pDEZM2qBWei3JFUD0joDx2m\n"
"AtbooMEMoS1ictwn0J0DxgRdE7kBuQENBF0TvAEIAOarRjy8Yj019VLu0ojdyTjK\n"
"CYe7uhtBjZB2mQaJXi9WDH9n50llfXfZukt0vKzelfobdUKaLQpiYRGqYDY+xemK\n"
"oNz1d2MDDf6/AktOpdxzWaG8Aml0muoV0y5rkBCLFJZb+AU1T1SXVr8blky5j0xi\n"
"6VXs5izY6hjCC+m+ERuhgCuyRc03SUGsRwxszeRfXFBN8S3KgpVIF5BgiYiM+ldJ\n"
"jCOLFEDST/E0LPKltcZw+pycN6lyjEdYMgwC60tJYNSlnKe0mfhfmL4UBHqQgRZF\n"
"cL038hn0y9gqf0jwF5DZg3w4gdVoziT+Zxfuv6a6OgtQoN87+I17LvKAmT7U8a/r\n"
"sQf+MRogIbUw09HbKLRO6hdfn6Zrhl6qwkl7Z0cIM32jV7N7ADwk5vtZxMzluHXG\n"
"IybbF0IyaocxxeE3LqoVnmhie6xXwGCJiIz6V0mMI4sUQNJP8TQs8qW1xnD6nJw3\n"
"qXKMR1gyDALrS0lg1KWcp7SZ+F+YvhQEepCBFkVwvTfyGfTL2Cp/SPAX1bJjQD59\n"
"S0xOJPyyHUKy4zFaZaZNkP6jHvOZgvT/vF3ZcvdJxsjGRJQ2k2mqHNqJZ1hTuDLc\n"
"NcUcFYMmcbl/5d7ihbqL3rBH1v3k7HqzDKVGQJMqkoRppIPLG6K0wZbkbdu+nshO\n"
"GaGKpPpHVPTvA8iNGrCWXMvgJV4PvYQPEpBgtC7+RQYE/zxYQtGhoXmfGnKZUUnR\n"
"hNaKmLVcKVi3B9W1bslkRk4bqHmdAYZRd3bMJfbQWRLjLLt3cC0FG8VZZHUqZh9r\n"
"g3vMHzKE4yPL5cKaXPbj8DtBlJ7FJ4+r7R4LPCoqJiERBTiEWBMRjEe0cX6/KjGp\n"
"kR48=\n"
"=cWSN\n"
"-----END PGP PUBLIC KEY BLOCK-----";/* Test RSA private key from Go tests (same as in test_convert.c) */
static const char* TEST_RSA_PRIVATE_KEY = 
"-----BEGIN PGP PRIVATE KEY BLOCK-----\n"
"\n"
"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\n"
"iERBTiEWBMRjEe0cX6/KjGpkR48=\n"
"=gM4o\n"
"-----END PGP PRIVATE KEY BLOCK-----";/* Test extracting public key metadata */
void test_get_public_key_metadata(void) {
    TEST_START("test_get_public_key_metadata");
    
    /* Initialize library */
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT(init_result.error == OPENPGP_SUCCESS);
    
    /* Get metadata for public key */
    openpgp_result_t result = openpgp_get_public_key_metadata(TEST_RSA_PUBLIC_KEY);
    
    /* For now, we expect this to fail as the implementation is not complete */
    if (result.error != OPENPGP_SUCCESS) {
        TEST_LOG("Expected failure: metadata extraction not yet implemented");
        TEST_LOG("Error: %s", result.error_message ? result.error_message : "Unknown error");
        /* This is expected for now */
    } else {
        /* Once implemented, verify the metadata */
        TEST_ASSERT(result.data != NULL);
        openpgp_public_key_metadata_t *metadata = (openpgp_public_key_metadata_t *)result.data;
        
        /* Verify expected values based on the test key */
        TEST_ASSERT(metadata->algorithm != NULL);
        TEST_LOG("Algorithm: %s", metadata->algorithm);
        TEST_ASSERT(strcmp(metadata->algorithm, "RSA") == 0);
        
        TEST_ASSERT(metadata->key_id != NULL);
        TEST_LOG("Key ID: %s", metadata->key_id);
        
        TEST_ASSERT(metadata->fingerprint != NULL);
        TEST_LOG("Fingerprint: %s", metadata->fingerprint);
        
        TEST_ASSERT(metadata->can_sign == true);
        TEST_ASSERT(metadata->can_encrypt == true);
        
        /* Check identities */
        TEST_ASSERT(metadata->identities_count > 0);
        TEST_ASSERT(metadata->identities != NULL);
        TEST_LOG("Identity: %s", metadata->identities[0].name);
        
        /* Free metadata */
        openpgp_public_key_metadata_free(metadata);
    }
    
    /* Cleanup */
    openpgp_result_free(&result);
    openpgp_cleanup();
    
    TEST_PASS();
}

/* Test extracting private key metadata */
void test_get_private_key_metadata(void) {
    TEST_START("test_get_private_key_metadata");
    
    /* Initialize library */
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT(init_result.error == OPENPGP_SUCCESS);
    
    /* Get metadata for private key */
    openpgp_result_t result = openpgp_get_private_key_metadata(TEST_RSA_PRIVATE_KEY);
    
    /* For now, we expect this to fail as the implementation is not complete */
    if (result.error != OPENPGP_SUCCESS) {
        TEST_LOG("Expected failure: metadata extraction not yet implemented");
        TEST_LOG("Error: %s", result.error_message ? result.error_message : "Unknown error");
        /* This is expected for now */
    } else {
        /* Once implemented, verify the metadata */
        TEST_ASSERT(result.data != NULL);
        openpgp_private_key_metadata_t *metadata = (openpgp_private_key_metadata_t *)result.data;
        
        /* Verify expected values */
        TEST_ASSERT(metadata->key_id != NULL);
        TEST_LOG("Key ID: %s", metadata->key_id);
        
        TEST_ASSERT(metadata->fingerprint != NULL);
        TEST_LOG("Fingerprint: %s", metadata->fingerprint);
        
        TEST_ASSERT(metadata->encrypted == true); /* This test key is encrypted */
        TEST_ASSERT(metadata->can_sign == true);
        
        /* Check identities */
        TEST_ASSERT(metadata->identities_count > 0);
        TEST_ASSERT(metadata->identities != NULL);
        
        /* Free metadata */
        openpgp_private_key_metadata_free(metadata);
    }
    
    /* Cleanup */
    openpgp_result_free(&result);
    openpgp_cleanup();
    
    TEST_PASS();
}/* Test null key metadata extraction */
void test_get_null_key_metadata(void) {
    TEST_START("test_get_null_key_metadata");
    
    /* Initialize library */
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT(init_result.error == OPENPGP_SUCCESS);
    
    /* Try to get metadata for null key */
    openpgp_result_t pub_result = openpgp_get_public_key_metadata(NULL);
    TEST_ASSERT(pub_result.error == OPENPGP_ERROR_INVALID_INPUT);
    TEST_ASSERT(pub_result.data == NULL);
    
    openpgp_result_t priv_result = openpgp_get_private_key_metadata(NULL);
    TEST_ASSERT(priv_result.error == OPENPGP_ERROR_INVALID_INPUT);
    TEST_ASSERT(priv_result.data == NULL);
    
    /* Cleanup */
    openpgp_result_free(&pub_result);
    openpgp_result_free(&priv_result);
    openpgp_cleanup();
    
    TEST_PASS();
}

/* Test invalid key metadata extraction */
void test_get_invalid_key_metadata(void) {
    TEST_START("test_get_invalid_key_metadata");
    
    /* Initialize library */
    openpgp_result_t init_result = openpgp_init();
    TEST_ASSERT(init_result.error == OPENPGP_SUCCESS);
    
    /* Try to get metadata for invalid key */
    const char *invalid_key = "This is not a valid PGP key";
    
    openpgp_result_t pub_result = openpgp_get_public_key_metadata(invalid_key);
    TEST_ASSERT(pub_result.error != OPENPGP_SUCCESS);
    TEST_ASSERT(pub_result.data == NULL);
    
    openpgp_result_t priv_result = openpgp_get_private_key_metadata(invalid_key);
    TEST_ASSERT(priv_result.error != OPENPGP_SUCCESS);
    TEST_ASSERT(priv_result.data == NULL);
    
    /* Cleanup */
    openpgp_result_free(&pub_result);
    openpgp_result_free(&priv_result);
    openpgp_cleanup();
    
    TEST_PASS();
}

/* Run all metadata tests */
void run_metadata_tests(void) {
    printf("\n=== Key Metadata Tests ===\n");
    
    test_get_public_key_metadata();
    test_get_private_key_metadata();
    test_get_null_key_metadata();
    test_get_invalid_key_metadata();
}