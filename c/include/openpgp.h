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
    OPENPGP_ERROR_SIZE_LIMIT = 11,    /* Data exceeds processing limits */
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

/*
 * Size Limitations
 * 
 * The library enforces the following size limits based on empirical 
 * testing with FlatCC serialization (Phase 7.6 findings):
 * 
 * - Message content (encryption/decryption): 2KB maximum
 * - Signature data: 3KB maximum  
 * - Key generation comments: 512 bytes maximum
 * - FlatBuffer serialization: 4KB maximum
 * 
 * Operations exceeding these limits will return OPENPGP_ERROR_SIZE_LIMIT
 * with a descriptive error message indicating the specific limit exceeded.
 */

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

/* Identity information in a key */
typedef struct {
    char *id;                       /* Identity ID */
    char *name;                     /* User name */
    char *email;                    /* Email address */
    char *comment;                  /* Comment field */
} openpgp_identity_t;

/* Public key metadata */
typedef struct {
    char *algorithm;                /* Algorithm name (e.g., "RSA", "ECDSA") */
    char *key_id;                   /* Full key ID */
    char *key_id_short;             /* Short key ID (last 8 hex chars) */
    char *creation_time;            /* Creation time in RFC3339 format */
    char *fingerprint;              /* Key fingerprint */
    char *key_id_numeric;           /* Numeric key ID */
    bool is_sub_key;                /* True if this is a subkey */
    bool can_sign;                  /* True if key can sign */
    bool can_encrypt;               /* True if key can encrypt */
    openpgp_identity_t *identities; /* Array of identities */
    size_t identities_count;        /* Number of identities */
    struct openpgp_public_key_metadata *sub_keys;  /* Array of subkeys */
    size_t sub_keys_count;          /* Number of subkeys */
} openpgp_public_key_metadata_t;

/* Private key metadata */
typedef struct {
    char *key_id;                   /* Full key ID */
    char *key_id_short;             /* Short key ID (last 8 hex chars) */
    char *creation_time;            /* Creation time in RFC3339 format */
    char *fingerprint;              /* Key fingerprint */
    char *key_id_numeric;           /* Numeric key ID */
    bool is_sub_key;                /* True if this is a subkey */
    bool encrypted;                 /* True if key is encrypted */
    bool can_sign;                  /* True if key can sign */
    openpgp_identity_t *identities; /* Array of identities */
    size_t identities_count;        /* Number of identities */
    struct openpgp_private_key_metadata *sub_keys; /* Array of subkeys */
    size_t sub_keys_count;          /* Number of subkeys */
} openpgp_private_key_metadata_t;

/* Verification result */
typedef struct {
    bool is_valid;                  /* True if signature is valid */
    char *signer_key_id;            /* Key ID of the signer */
    char *signer_fingerprint;       /* Fingerprint of the signer */
    char *original_data;            /* Original data (for signed messages) */
    size_t original_data_len;       /* Length of original data */
    char *signature_time;           /* Time signature was created (RFC3339 format) */
    char *error_details;            /* Detailed error message if verification failed */
} openpgp_verification_result_t;

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
void openpgp_keypair_free(openpgp_keypair_t *keypair);

/**
 * Free an openpgp_public_key_metadata_t structure and its contents.
 * 
 * @param metadata The metadata to free
 */
void openpgp_public_key_metadata_free(openpgp_public_key_metadata_t *metadata);

/**
 * Free an openpgp_private_key_metadata_t structure and its contents.
 * 
 * @param metadata The metadata to free
 */
void openpgp_private_key_metadata_free(openpgp_private_key_metadata_t *metadata);

/**
 * Free an openpgp_verification_result_t structure and its contents.
 * 
 * @param result The verification result to free
 */
void openpgp_verification_result_free(openpgp_verification_result_t *result);/*
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
 * Key Operations
 */

/**
 * Convert a private key to its corresponding public key.
 * 
 * @param private_key The private key in ASCII armor format
 * @return Result containing the public key string on success
 */
openpgp_result_t openpgp_convert_private_to_public(const char *private_key);

/**
 * Get metadata for a public key.
 * 
 * @param public_key The public key in ASCII armor format
 * @return Result containing openpgp_public_key_metadata_t on success
 */
openpgp_result_t openpgp_get_public_key_metadata(const char *public_key);

/**
 * Get metadata for a private key.
 * 
 * @param private_key The private key in ASCII armor format
 * @return Result containing openpgp_private_key_metadata_t on success
 */
openpgp_result_t openpgp_get_private_key_metadata(const char *private_key);

/*
 * Symmetric Encryption Operations
 */

/**
 * Encrypt a message using symmetric encryption with a passphrase.
 * 
 * @param message The message to encrypt
 * @param passphrase The passphrase for encryption
 * @param file_hints File hints for the encrypted data (can be NULL)
 * @param options Key options for encryption (can be NULL for defaults)
 * @return Result containing the encrypted message string on success
 */
openpgp_result_t openpgp_encrypt_symmetric(const char *message, const char *passphrase, 
                                          const openpgp_file_hints_t *file_hints,
                                          const openpgp_key_options_t *options);

/**
 * Decrypt a message using symmetric decryption with a passphrase.
 * 
 * @param message The encrypted message to decrypt
 * @param passphrase The passphrase for decryption
 * @param options Key options for decryption (can be NULL for defaults)
 * @return Result containing the decrypted message string on success
 */
openpgp_result_t openpgp_decrypt_symmetric(const char *message, const char *passphrase,
                                          const openpgp_key_options_t *options);

/**
 * Encrypt a file using symmetric encryption with a passphrase.
 * 
 * @param input_file Path to the input file to encrypt
 * @param output_file Path to the output encrypted file
 * @param passphrase The passphrase for encryption
 * @param file_hints File hints for the encrypted data (can be NULL)
 * @param options Key options for encryption (can be NULL for defaults)
 * @return Result with success/error status
 */
openpgp_result_t openpgp_encrypt_symmetric_file(const char *input_file, const char *output_file,
                                               const char *passphrase,
                                               const openpgp_file_hints_t *file_hints,
                                               const openpgp_key_options_t *options);

/**
 * Decrypt a file using symmetric decryption with a passphrase.
 * 
 * @param input_file Path to the encrypted input file
 * @param output_file Path to the output decrypted file
 * @param passphrase The passphrase for decryption
 * @param options Key options for decryption (can be NULL for defaults)
 * @return Result with success/error status
 */
openpgp_result_t openpgp_decrypt_symmetric_file(const char *input_file, const char *output_file,
                                               const char *passphrase,
                                               const openpgp_key_options_t *options);

/**
 * Encrypt binary data using symmetric encryption with a passphrase.
 * 
 * @param data The binary data to encrypt
 * @param data_len Length of the binary data
 * @param passphrase The passphrase for encryption
 * @param file_hints File hints for the encrypted data (can be NULL)
 * @param options Key options for encryption (can be NULL for defaults)
 * @return Result containing the encrypted data on success
 */
openpgp_result_t openpgp_encrypt_symmetric_bytes(const uint8_t *data, size_t data_len,
                                                const char *passphrase,
                                                const openpgp_file_hints_t *file_hints,
                                                const openpgp_key_options_t *options);

/**
 * Decrypt binary data using symmetric decryption with a passphrase.
 * 
 * @param data The encrypted binary data to decrypt
 * @param data_len Length of the encrypted binary data
 * @param passphrase The passphrase for decryption
 * @param options Key options for decryption (can be NULL for defaults)
 * @return Result containing the decrypted data on success
 */
openpgp_result_t openpgp_decrypt_symmetric_bytes(const uint8_t *data, size_t data_len,
                                                const char *passphrase,
                                                const openpgp_key_options_t *options);

/*
 * Asymmetric Encryption Operations
 */

/**
 * Encrypt a message using asymmetric encryption with recipient public keys.
 * 
 * @param message The message to encrypt
 * @param recipient_keys Array of recipient public keys in ASCII armor format
 * @param recipient_count Number of recipient keys
 * @param options Key options for encryption (can be NULL for defaults)
 * @return Result containing the encrypted message string on success
 */
openpgp_result_t openpgp_encrypt(const char *message, 
                                const char **recipient_keys,
                                size_t recipient_count,
                                const openpgp_key_options_t *options);

/**
 * Decrypt a message using asymmetric decryption with a private key.
 * 
 * @param message The encrypted message to decrypt
 * @param private_key The private key for decryption in ASCII armor format
 * @param passphrase The passphrase for the private key (can be NULL if unencrypted)
 * @param options Key options for decryption (can be NULL for defaults)
 * @return Result containing the decrypted message string on success
 */
openpgp_result_t openpgp_decrypt(const char *message,
                                const char *private_key,
                                const char *passphrase,
                                const openpgp_key_options_t *options);

/**
 * Encrypt a file using asymmetric encryption with recipient public keys.
 * 
 * @param input_file Path to the input file to encrypt
 * @param output_file Path to the output encrypted file
 * @param recipient_keys Array of recipient public keys in ASCII armor format
 * @param recipient_count Number of recipient keys
 * @param file_hints File hints for the encrypted data (can be NULL)
 * @param options Key options for encryption (can be NULL for defaults)
 * @return Result containing success status
 */
openpgp_result_t openpgp_encrypt_file(const char *input_file,
                                     const char *output_file,
                                     const char **recipient_keys,
                                     size_t recipient_count,
                                     const openpgp_file_hints_t *file_hints,
                                     const openpgp_key_options_t *options);

/**
 * Decrypt a file using asymmetric decryption with a private key.
 * 
 * @param input_file Path to the encrypted input file
 * @param output_file Path to the decrypted output file
 * @param private_key The private key for decryption in ASCII armor format
 * @param passphrase The passphrase for the private key (can be NULL if unencrypted)
 * @param options Key options for decryption (can be NULL for defaults)
 * @return Result containing success status
 */
openpgp_result_t openpgp_decrypt_file(const char *input_file,
                                     const char *output_file,
                                     const char *private_key,
                                     const char *passphrase,
                                     const openpgp_key_options_t *options);

/**
 * Encrypt binary data using asymmetric encryption with recipient public keys.
 * 
 * @param data The binary data to encrypt
 * @param data_len Length of the binary data
 * @param recipient_keys Array of recipient public keys in ASCII armor format
 * @param recipient_count Number of recipient keys
 * @param file_hints File hints for the encrypted data (can be NULL)
 * @param options Key options for encryption (can be NULL for defaults)
 * @return Result containing the encrypted data on success
 */
openpgp_result_t openpgp_encrypt_bytes(const uint8_t *data, size_t data_len,
                                      const char **recipient_keys,
                                      size_t recipient_count,
                                      const openpgp_file_hints_t *file_hints,
                                      const openpgp_key_options_t *options);

/**
 * Decrypt binary data using asymmetric decryption with a private key.
 * 
 * @param data The encrypted binary data to decrypt
 * @param data_len Length of the encrypted binary data
 * @param private_key The private key for decryption in ASCII armor format
 * @param passphrase The passphrase for the private key (can be NULL if unencrypted)
 * @param options Key options for decryption (can be NULL for defaults)
 * @return Result containing the decrypted data on success
 */
openpgp_result_t openpgp_decrypt_bytes(const uint8_t *data, size_t data_len,
                                      const char *private_key,
                                      const char *passphrase,
                                      const openpgp_key_options_t *options);

/*
 * Signing Operations
 */

/**
 * Sign a message using a private key.
 * 
 * @param message The message to sign
 * @param private_key The private key for signing in ASCII armor format
 * @param passphrase The passphrase for the private key (can be NULL if unencrypted)
 * @param options Key options for signing (can be NULL for defaults)
 * @return Result containing the signature string on success
 */
openpgp_result_t openpgp_sign(const char *message,
                             const char *private_key,
                             const char *passphrase,
                             const openpgp_key_options_t *options);

/**
 * Sign a message (alternative format) using a private key.
 * 
 * @param message The message to sign
 * @param private_key The private key for signing in ASCII armor format
 * @param passphrase The passphrase for the private key (can be NULL if unencrypted)
 * @param options Key options for signing (can be NULL for defaults)
 * @return Result containing the signature string on success
 */
openpgp_result_t openpgp_sign_data(const char *message,
                                  const char *private_key,
                                  const char *passphrase,
                                  const openpgp_key_options_t *options);

/**
 * Sign a file using a private key.
 * 
 * @param input_file Path to the input file to sign
 * @param private_key The private key for signing in ASCII armor format
 * @param passphrase The passphrase for the private key (can be NULL if unencrypted)
 * @param options Key options for signing (can be NULL for defaults)
 * @return Result containing the signature string on success
 */
openpgp_result_t openpgp_sign_file(const char *input_file,
                                  const char *private_key,
                                  const char *passphrase,
                                  const openpgp_key_options_t *options);

/**
 * Sign binary data using a private key.
 * 
 * @param data The binary data to sign
 * @param data_len Length of the binary data
 * @param private_key The private key for signing in ASCII armor format
 * @param passphrase The passphrase for the private key (can be NULL if unencrypted)
 * @param options Key options for signing (can be NULL for defaults)
 * @return Result containing the signature string on success
 */
openpgp_result_t openpgp_sign_bytes(const uint8_t *data, size_t data_len,
                                   const char *private_key,
                                   const char *passphrase,
                                   const openpgp_key_options_t *options);

/**
 * Sign binary data (alternative format) using a private key.
 * 
 * @param data The binary data to sign
 * @param data_len Length of the binary data
 * @param private_key The private key for signing in ASCII armor format
 * @param passphrase The passphrase for the private key (can be NULL if unencrypted)
 * @param options Key options for signing (can be NULL for defaults)
 * @return Result containing the signature string on success
 */
openpgp_result_t openpgp_sign_data_bytes(const uint8_t *data, size_t data_len,
                                        const char *private_key,
                                        const char *passphrase,
                                        const openpgp_key_options_t *options);

/*
 * Verification Operations
 */

/**
 * Verify a signed message and extract original content.
 * 
 * @param signed_message The signed message in ASCII armor format
 * @param public_key The public key to verify against in ASCII armor format
 * @param result Output parameter for verification details (allocated by function)
 * @return Operation result with error code
 * 
 * Example:
 *   openpgp_verification_result_t* result = NULL;
 *   openpgp_result_t res = openpgp_verify(signed_msg, pub_key, &result);
 *   if (res.error == OPENPGP_SUCCESS && result->is_valid) {
 *       printf("Original: %s\n", result->original_data);
 *   }
 *   openpgp_verification_result_free(result);
 */
openpgp_result_t openpgp_verify(const char *signed_message,
                               const char *public_key,
                               openpgp_verification_result_t **result);

/**
 * Verify a detached signature against data.
 * 
 * @param data The original data that was signed
 * @param data_len Length of the data
 * @param signature The detached signature in ASCII armor format
 * @param public_key The public key to verify against in ASCII armor format
 * @param result Output parameter for verification details (allocated by function)
 * @return Operation result with error code
 */
openpgp_result_t openpgp_verify_data(const void *data,
                                    size_t data_len,
                                    const char *signature,
                                    const char *public_key,
                                    openpgp_verification_result_t **result);

/**
 * Verify a file signature.
 * 
 * @param file_path Path to the file that was signed
 * @param signature The signature (inline or detached) in ASCII armor format
 * @param public_key The public key to verify against in ASCII armor format
 * @param result Output parameter for verification details (allocated by function)
 * @return Operation result with error code
 */
openpgp_result_t openpgp_verify_file(const char *file_path,
                                    const char *signature,
                                    const char *public_key,
                                    openpgp_verification_result_t **result);

/**
 * Verify a signature against raw binary data.
 * 
 * @param data The binary data that was signed
 * @param data_len Length of the binary data
 * @param signature The signature in ASCII armor format
 * @param public_key The public key to verify against in ASCII armor format
 * @param result Output parameter for verification details (allocated by function)
 * @return Operation result with error code
 */
openpgp_result_t openpgp_verify_bytes(const uint8_t *data,
                                     size_t data_len,
                                     const char *signature,
                                     const char *public_key,
                                     openpgp_verification_result_t **result);

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