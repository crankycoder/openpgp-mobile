# OpenPGP C Wrapper Library

This is a C wrapper library for the OpenPGP Mobile Go implementation. It provides a user-friendly C API that communicates with the Go library through FlatBuffers.

## Architecture

```
C Application -> C Wrapper API -> FlatBuffers -> OpenPGPBridgeCall -> Go OpenPGP
```

## Building

```bash
# Build both static and shared libraries
make

# Run tests
make test

# Clean build artifacts
make clean
```

## Usage

```c
#include "openpgp.h"

// Initialize the library
openpgp_result_t result = openpgp_init();
if (result.error != OPENPGP_SUCCESS) {
    fprintf(stderr, "Failed to initialize: %s\n", result.error_message);
    return 1;
}

// Use the library...

// Cleanup
openpgp_cleanup();
```

## API Reference

See `include/openpgp.h` for the complete API documentation.

## Dependencies

- libopenpgp_bridge.so (from the Go build)
- FlatBuffers C library (automatically handled)

## Error Handling

All functions return `openpgp_result_t` which contains:
- `error`: Error code (OPENPGP_SUCCESS on success)
- `error_message`: Human-readable error description
- `data`: Result data (function-specific)

Always check the error code before using the result data.