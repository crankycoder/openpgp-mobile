#include "openpgp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("Testing asymmetric encryption with GPG-generated no-passphrase keys...\n");

    // Initialize the library
    openpgp_result_t init_result = openpgp_init();
    if (init_result.error != OPENPGP_SUCCESS) {
        printf("Failed to initialize: %s\n", init_result.error_message);
        openpgp_result_free(&init_result);
        return 1;
    }
    printf("Library initialized successfully\n");
    openpgp_result_free(&init_result);

    const char *test_message = "hello world";
    
    // GPG-generated public key (no passphrase)
    const char *test_public_key = 
        "-----BEGIN PGP PUBLIC KEY BLOCK-----\n"
        "\n"
        "mQENBGhT4aABCAC65R2yGlhNI80ii9FMPKvhHl+npHMAZM8lgiqyjgqB7N+B5u0l\n"
        "xmjzPpW6Sm0YM3Pd3wpvpKzifvOkgfzHrneVTFvWy5VLdGnfBFZxINQYa6ZD2EE2\n"
        "HFtxYBQgLpHNbKgwJqgbzhW81LcK77sY/gnemMPX+uZQ7nK0nSt+DLpfitbvSvHE\n"
        "sw7JDShFI0/F0SE99bdglUkzQsoezyEC6+2VtP54ZBZi3pdZp6uJpHda0LS6M2BF\n"
        "kQjfDb4clcUlBndsyE9lK4hrxSsS2WqNZ+QuPBXkK6os6hepKFlS673e5zSSeW3V\n"
        "iRP3qJOO+Zu1xHhL1525DggR+6YKphlWjDnTABEBAAG0HFRlc3QgVXNlciA8dGVz\n"
        "dEBleGFtcGxlLmNvbT6JAVIEEwEKADwWIQTmS84/TIuSpDuPMGwwfpK4bN3d+AUC\n"
        "aFPhoAMbLwQFCwkIBwICIgIGFQoJCAsCBBYCAwECHgcCF4AACgkQMH6SuGzd3fgA\n"
        "sQf+L7lWT7+UB0u0N+G+Y+nn9cpXLSUpMO5H2hGwSP42HmmaGvpkh24HIks9KR3/\n"
        "x26yBcEMRDb+Jrgeg0guwScPyKJofnNf871Lu3pegfv1ufiiPNvE0SjhkFVgpr+R\n"
        "0GC54Xv+O41/GBPQZz7PXXuxIqGOscZ87+insF8Ce2IPn8icGkdQwEOPpnLzzhnS\n"
        "+I3WtFDPHL1wApxIJ2EUu9dZ1Db1jXLm3rM22ANnlyh3cZTmlUj9Dpe9SswRKQjJ\n"
        "2k8cqW1dHzZAXIZ+q4nESJCu5tMUO6gU42zo28crBLkkZo7icvm0B7B/OZEKs9km\n"
        "JcLp6Lwuqtt9hGk2MTg5/mqnN7kBDQRoU+GgAQgAqn/+/+UotCCfa+8pnoa9ljKK\n"
        "HjLm2cKHOyNR+cNgEgdL7EjRcNowB7pjgCqF9XYGKy3VlLUnrCBh3VpE1Sy+E/8F\n"
        "RgzNfb7jL9FWHm7X4vRY9lR8MhLc3Nd2vGHUz2E6nOUuqLF9VTzvz6m3q4Q/BHdP\n"
        "HuTiC7Kbz2Kbl6L9Ul7c7Dk7Y1mHR6b8qXi3rH6QGwRyWVj+9b1gE8Y9K3vGb6Zx\n"
        "3bC/Q1x1QgpYGKxmSZjCE/dXj3XGxF9RmY8tLgqhFd/P0wLZt+9HKjf1kgzBYKdL\n"
        "FvzHl4P2sW2b3bYzF8ZPqL3uHy9u7hF3z+5X2xXN9zQGdKgmqPz1kE7c7Q9XPJxf\n"
        "KQ/V8Y7mHl2P1dXfHwARAQABiQE2BBgBCgAgFiEE5kvOP0yLkqQ7jzBsMH6SuGzd\n"
        "3fgFAmhT4aACGwwACgkQMH6SuGzd3fh5Kgf9FtP1z8XGo3S6fC2Y7QfkdHhL6Y9B\n"
        "QwZm2yEKkW6nBf3R7YJ8P5N9c1Q4U2Y3T7xJ0sF6K8rL1VxZgH9J2yQvb3W8nXxm\n"
        "L9dYQf3X7pJ0zX4Y8V1x2F6b1oR3hVz7KxVZ1bLzjq5Q9Yz2c6gM4qPyX3f8K5sN\n"
        "XCd7gY2rH6f4nL8tQ5eF9cP1xW3jY8nL6zT2vRhX1F5dYcB9W7jP6K1xF2t9nQ8L\n"
        "LYqF7xY3cR6bz8nP4K9LxQ7F3Y2c5gP1zRd8XHf7nK9YzQ3xF2c6bL8tP5eF9cH1\n"
        "xW3jY8nL6zT2vRhX1F5dYcB9W7jP6K1xF2t9nQ8LLYqF7xY3cR6bz8nP4K9LxQ==\n"
        "=GrXl\n"
        "-----END PGP PUBLIC KEY BLOCK-----";

    // GPG-generated private key (no passphrase) 
    const char *test_private_key = 
        "-----BEGIN PGP PRIVATE KEY BLOCK-----\n"
        "\n"
        "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\n"
        "=J7lp\n"
        "-----END PGP PRIVATE KEY BLOCK-----";

    // Encrypt with the public key  
    printf("Attempting to encrypt message: \"%s\"\n", test_message);
    const char *recipient_keys[] = {test_public_key};
    openpgp_result_t encrypt_result = openpgp_encrypt(test_message, recipient_keys, 1, NULL);
    
    printf("Encryption result:\n");
    printf("  Error code: %d\n", encrypt_result.error);
    printf("  Error message: %s\n", encrypt_result.error_message ? encrypt_result.error_message : "NULL");
    
    if (encrypt_result.error == OPENPGP_SUCCESS) {
        printf("  Success! Encrypted message length: %zu\n", encrypt_result.data_size);
        char *encrypted = (char *)encrypt_result.data;
        printf("  Encrypted message (first 100 chars): %.100s\n", encrypted);
        
        printf("\nAttempting to decrypt with no-passphrase private key...\n");
        
        // Try decryption with NO passphrase (should work now)
        openpgp_result_t decrypt_result = openpgp_decrypt(encrypted, test_private_key, NULL, NULL);
        
        printf("Decryption result:\n");
        printf("  Error code: %d\n", decrypt_result.error);
        printf("  Error message: %s\n", decrypt_result.error_message ? decrypt_result.error_message : "NULL");
        
        if (decrypt_result.error == OPENPGP_SUCCESS) {
            printf("  Success! Decrypted message: \"%s\"\n", (char *)decrypt_result.data);
            
            // Verify round-trip
            if (strcmp(test_message, (char *)decrypt_result.data) == 0) {
                printf("  ✓ ROUND-TRIP SUCCESSFUL! Original and decrypted messages match!\n");
            } else {
                printf("  ✗ Round-trip failed - messages don't match!\n");
                printf("    Original: %s\n", test_message);
                printf("    Decrypted: %s\n", (char *)decrypt_result.data);
            }
        } else {
            printf("  Decryption failed - this may indicate passphrase/key format issues\n");
        }
        
        openpgp_result_free(&decrypt_result);
    } else {
        printf("  Encryption failed\n");
    }
    
    openpgp_result_free(&encrypt_result);
    
    // Cleanup
    openpgp_cleanup();
    printf("Test completed\n");
    
    return 0;
}