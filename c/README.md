# OpenPGP C Binding

This directory contains a C wrapper library for the OpenPGP Mobile library. It provides a user-friendly C API that communicates with the Go implementation via FlatBuffers.

## Architecture

```
C Application → C Wrapper API → FlatBuffers → OpenPGPBridgeCall → Go OpenPGP
```

## Building

```bash
# Generate FlatBuffer headers
make -C .. flatbuffers_c_wrapper

# Build the C library
make

# Run tests
make test

# Clean build artifacts
make clean
```

## Key Operations

### Converting Private Key to Public Key

```c
#include "openpgp.h"

// Initialize the library
openpgp_result_t init_result = openpgp_init();
if (init_result.error != OPENPGP_SUCCESS) {
    // Handle error
}

// Convert private key to public key
openpgp_result_t result = openpgp_convert_private_to_public(private_key_pem);
if (result.error == OPENPGP_SUCCESS) {
    char *public_key = (char *)result.data;
    // Use the public key
    printf("Public key: %s\n", public_key);
}

// Clean up
openpgp_result_free(&result);
openpgp_cleanup();
```

### Extracting Public Key Metadata

```c
// Get metadata from a public key
openpgp_result_t result = openpgp_get_public_key_metadata(public_key_pem);
if (result.error == OPENPGP_SUCCESS) {
    openpgp_public_key_metadata_t *metadata = 
        (openpgp_public_key_metadata_t *)result.data;
    
    printf("Algorithm: %s\n", metadata->algorithm);
    printf("Key ID: %s\n", metadata->key_id);
    printf("Fingerprint: %s\n", metadata->fingerprint);
    printf("Can Sign: %s\n", metadata->can_sign ? "Yes" : "No");
    printf("Can Encrypt: %s\n", metadata->can_encrypt ? "Yes" : "No");
    
    // Print identities
    for (size_t i = 0; i < metadata->identities_count; i++) {
        printf("Identity: %s <%s>\n", 
               metadata->identities[i].name,
               metadata->identities[i].email);
    }
    
    // Free the metadata
    openpgp_public_key_metadata_free(metadata);
}
openpgp_result_free(&result);
```

### Extracting Private Key Metadata

```c
// Get metadata from a private key
openpgp_result_t result = openpgp_get_private_key_metadata(private_key_pem);
if (result.error == OPENPGP_SUCCESS) {
    openpgp_private_key_metadata_t *metadata = 
        (openpgp_private_key_metadata_t *)result.data;
    
    printf("Key ID: %s\n", metadata->key_id);
    printf("Fingerprint: %s\n", metadata->fingerprint);
    printf("Encrypted: %s\n", metadata->encrypted ? "Yes" : "No");
    printf("Can Sign: %s\n", metadata->can_sign ? "Yes" : "No");
    
    // Free the metadata
    openpgp_private_key_metadata_free(metadata);
}
openpgp_result_free(&result);
```

## Memory Management

The C binding follows these memory management rules:

1. **Initialization**: Call `openpgp_init()` before using any other functions
2. **Results**: All functions return `openpgp_result_t` which must be freed with `openpgp_result_free()`
3. **Data Structures**: Use the appropriate free functions:
   - `openpgp_keypair_free()` for key pairs
   - `openpgp_public_key_metadata_free()` for public key metadata
   - `openpgp_private_key_metadata_free()` for private key metadata
4. **Cleanup**: Call `openpgp_cleanup()` when done with the library

## Error Handling

All functions return an `openpgp_result_t` structure:

```c
typedef struct {
    openpgp_error_t error;      // Error code (OPENPGP_SUCCESS on success)
    char *error_message;        // Human-readable error description
    void *data;                 // Operation-specific result data
    size_t data_size;           // Size of result data
} openpgp_result_t;
```

Common error codes:
- `OPENPGP_SUCCESS` - Operation succeeded
- `OPENPGP_ERROR_INVALID_INPUT` - Invalid input parameters
- `OPENPGP_ERROR_LIBRARY_NOT_INITIALIZED` - Library not initialized
- `OPENPGP_ERROR_BRIDGE_CALL` - Bridge operation failed
- `OPENPGP_ERROR_SERIALIZATION` - FlatBuffer serialization error
- `OPENPGP_ERROR_MEMORY_ALLOCATION` - Memory allocation failed

## Examples

See the `examples/` directory for complete working examples:
- `generate_key.c` - Key generation example
- `key_operations.c` - Key conversion and metadata extraction
- `symmetric_encryption.c` - Symmetric encryption operations

## Implementation Status

### Completed (Phase 1, 2 & 3)
- ✅ Key generation (RSA, ECDSA, EdDSA)
- ✅ Private to public key conversion
- ✅ Public key metadata extraction
- ✅ Private key metadata extraction
- ✅ **Symmetric encryption/decryption (Phase 3)**
  - ✅ Message encryption/decryption with passphrase
  - ✅ File-to-file encryption/decryption
  - ✅ Binary data encryption/decryption
  - ✅ Support for custom encryption options (cipher, hash, compression)
  - ✅ Support for file hints (filename, modification time, binary flag)

### Planned
- ⏳ Asymmetric encryption/decryption (Phase 4)
- ⏳ Signing operations (Phase 5)
- ⏳ Verification operations (Phase 6)
- ⏳ Combined operations (Phase 7)

## Technical Details

### FlatBuffer Integration
The C binding uses FlatBuffers for serialization when communicating with the Go bridge. This ensures:
- Type safety across language boundaries
- Efficient serialization/deserialization
- Clear message structure definitions

### Bridge Communication
All operations go through the `OpenPGPBridgeCall` function which:
1. Accepts FlatBuffer-serialized requests
2. Processes them in the Go implementation
3. Returns FlatBuffer-serialized responses

### Thread Safety
The current implementation is not thread-safe. Use appropriate synchronization if calling from multiple threads.