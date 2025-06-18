#ifndef OPENPGP_H
#define OPENPGP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * OpenPGP C Wrapper Library
 *
 * This library provides a user-friendly C API for OpenPGP operations.
 * It communicates with the Go OpenPGP implementation via FlatBuffers.
 *
 * Architecture:
 * C Application -> C Wrapper API -> FlatBuffers -> OpenPGPBridgeCall -> Go OpenPGP
 */

/* Version information */
#define OPENPGP_VERSION_MAJOR 1
#define OPENPGP_VERSION_MINOR 0
#define OPENPGP_VERSION_PATCH 0
#define OPENPGP_VERSION "1.0.0"

/* Error codes */
typedef enum {
    OPENPGP_SUCCESS = 0,
    OPENPGP_ERROR_INVALID_INPUT = 1,
    OPENPGP_ERROR_ENCRYPTION_FAILED = 2,
    OPENPGP_ERROR_DECRYPTION_FAILED = 3,
    OPENPGP_ERROR_SIGNING_FAILED = 4,
    OPENPGP_ERROR_VERIFICATION_FAILED = 5,
    OPENPGP_ERROR_KEY_GENERATION_FAILED = 6,
    OPENPGP_ERROR_MEMORY_ALLOCATION = 7,
    OPENPGP_ERROR_SERIALIZATION = 8,
    OPENPGP_ERROR_BRIDGE_CALL = 9,
    OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED = 10,
    OPENPGP_ERROR_UNKNOWN = 99
} openpgp_error_t;

/* Algorithms */
typedef enum {
    OPENPGP_ALGORITHM_RSA = 0,
    OPENPGP_ALGORITHM_ECDSA = 1,
    OPENPGP_ALGORITHM_EDDSA = 2,
    OPENPGP_ALGORITHM_ECHD = 3,
    OPENPGP_ALGORITHM_DSA = 4,
    OPENPGP_ALGORITHM_ELGAMAL = 5
} openpgp_algorithm_t;

/* ECC Curves */
typedef enum {
    OPENPGP_CURVE_CURVE25519 = 0,
    OPENPGP_CURVE_CURVE448 = 1,
    OPENPGP_CURVE_P256 = 2,
    OPENPGP_CURVE_P384 = 3,
    OPENPGP_CURVE_P521 = 4,
    OPENPGP_CURVE_SECP256K1 = 5,
    OPENPGP_CURVE_BRAINPOOLP256 = 6,
    OPENPGP_CURVE_BRAINPOOLP384 = 7,
    OPENPGP_CURVE_BRAINPOOLP512 = 8
} openpgp_curve_t;

/* Hash algorithms */
typedef enum {
    OPENPGP_HASH_SHA256 = 0,
    OPENPGP_HASH_SHA224 = 1,
    OPENPGP_HASH_SHA384 = 2,
    OPENPGP_HASH_SHA512 = 3
} openpgp_hash_t;

/* Compression algorithms */
typedef enum {
    OPENPGP_COMPRESSION_NONE = 0,
    OPENPGP_COMPRESSION_ZLIB = 1,
    OPENPGP_COMPRESSION_ZIP = 2
} openpgp_compression_t;

/* Cipher algorithms */
typedef enum {
    OPENPGP_CIPHER_AES128 = 0,
    OPENPGP_CIPHER_AES192 = 1,
    OPENPGP_CIPHER_AES256 = 2,
    OPENPGP_CIPHER_DES = 3,
    OPENPGP_CIPHER_CAST5 = 4
} openpgp_cipher_t;/* Result structure for all operations */
typedef struct {
    openpgp_error_t error;          /* Error code (OPENPGP_SUCCESS on success) */
    char *error_message;            /* Human-readable error description (NULL on success) */
    void *data;                     /* Operation-specific result data */
    size_t data_size;               /* Size of result data in bytes */
} openpgp_result_t;

/* Key options */
typedef struct {
    openpgp_algorithm_t algorithm;
    openpgp_curve_t curve;
    openpgp_hash_t hash;
    openpgp_cipher_t cipher;
    openpgp_compression_t compression;
    int compression_level;          /* -1 to 9, -1 = default */
    int rsa_bits;                   /* RSA key size, 0 = default (2048) */
} openpgp_key_options_t;

/* Options for key generation */
typedef struct {
    const char *name;               /* User name */
    const char *comment;            /* Comment field */
    const char *email;              /* Email address */
    const char *passphrase;         /* Passphrase (NULL for no passphrase) */
    openpgp_key_options_t key_options;
} openpgp_options_t;

/* Key pair result */
typedef struct {
    char *public_key;               /* PGP public key in ASCII armor */
    char *private_key;              /* PGP private key in ASCII armor */
} openpgp_keypair_t;

/* File hints for encryption/signing */
typedef struct {
    bool is_binary;                 /* True if file is binary */
    const char *file_name;          /* Original filename (NULL if not applicable) */
    const char *mod_time;           /* Modification time in RFC3339 format (NULL if not applicable) */
} openpgp_file_hints_t;

/* Entity for signing operations */
typedef struct {
    const char *public_key;         /* Public key for verification */
    const char *private_key;        /* Private key for signing */
    const char *passphrase;         /* Passphrase for private key */
} openpgp_entity_t;

/*
 * Library Initialization and Cleanup
 */

/**
 * Initialize the OpenPGP library.
 * Must be called before any other operations.
 * 
 * @return Result with error code
 */
openpgp_result_t openpgp_init(void);

/**
 * Cleanup the OpenPGP library.
 * Should be called when done with the library.
 */
void openpgp_cleanup(void);

/*
 * Memory Management
 */

/**
 * Free an openpgp_result_t structure and its contents.
 * 
 * @param result The result to free
 */
void openpgp_result_free(openpgp_result_t *result);

/**
 * Free an openpgp_keypair_t structure and its contents.
 * 
 * @param keypair The keypair to free
 */
void openpgp_keypair_free(openpgp_keypair_t *keypair);/*
 * Key Generation
 */

/**
 * Generate a new OpenPGP key pair with default options.
 * 
 * @param name User name
 * @param email Email address
 * @param passphrase Passphrase (NULL for no passphrase)
 * @return Result containing openpgp_keypair_t on success
 */
openpgp_result_t openpgp_generate_key(const char *name, const char *email, const char *passphrase);

/**
 * Generate a new OpenPGP key pair with custom options.
 * 
 * @param options Key generation options
 * @return Result containing openpgp_keypair_t on success
 */
openpgp_result_t openpgp_generate_key_with_options(const openpgp_options_t *options);

/*
 * Helper Functions
 */

/**
 * Get a human-readable string for an error code.
 * 
 * @param error The error code
 * @return String description of the error
 */
const char *openpgp_error_string(openpgp_error_t error);

/**
 * Initialize options structure with default values.
 * 
 * @param options Options structure to initialize
 */
void openpgp_options_init_default(openpgp_options_t *options);

/**
 * Initialize key options structure with default values.
 * 
 * @param key_options Key options structure to initialize
 */
void openpgp_key_options_init_default(openpgp_key_options_t *key_options);

#ifdef __cplusplus
}
#endif

#endif /* OPENPGP_H */