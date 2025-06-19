#ifndef TEST_FIXTURES_H
#define TEST_FIXTURES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Utility functions for loading test fixture data from files
 * instead of embedding keys in C code.
 */

/**
 * Read entire file contents into a dynamically allocated string.
 * Caller must free the returned pointer.
 * 
 * @param filepath Path to the file to read
 * @return Dynamically allocated string with file contents, or NULL on error
 */
static char* read_fixture_file(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("Error: Could not open fixture file: %s\n", filepath);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer
    char* buffer = malloc(file_size + 1);
    if (!buffer) {
        printf("Error: Could not allocate memory for fixture file\n");
        fclose(file);
        return NULL;
    }
    
    // Read file
    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';
    
    fclose(file);
    return buffer;
}

/**
 * Load test public key (no passphrase) from fixture file.
 * Caller must free the returned pointer.
 */
static char* load_test_public_key_no_passphrase(void) {
    return read_fixture_file("test/fixtures/test-public-key-no-passphrase.asc");
}

/**
 * Load test private key (no passphrase) from fixture file.
 * Caller must free the returned pointer.
 */
static char* load_test_private_key_no_passphrase(void) {
    return read_fixture_file("test/fixtures/test-private-key-no-passphrase.asc");
}

/**
 * Load test message from fixture file.
 * Caller must free the returned pointer.
 */
static char* load_test_message(void) {
    return read_fixture_file("test/fixtures/test-message.txt");
}

/**
 * Load test public key (with passphrase protection) from fixture file.
 * Caller must free the returned pointer.
 */
static char* load_test_public_key_with_passphrase(void) {
    return read_fixture_file("test/fixtures/test-public-key-with-passphrase.asc");
}

/**
 * Load test private key (with passphrase protection) from fixture file.
 * Caller must free the returned pointer.
 */
static char* load_test_private_key_with_passphrase(void) {
    return read_fixture_file("test/fixtures/test-private-key-with-passphrase.asc");
}

/**
 * Load test passphrase from fixture file.
 * Caller must free the returned pointer.
 */
static char* load_test_passphrase(void) {
    char* passphrase = read_fixture_file("test/fixtures/test-passphrase.txt");
    if (passphrase) {
        // Remove trailing newline if present
        size_t len = strlen(passphrase);
        if (len > 0 && passphrase[len-1] == '\n') {
            passphrase[len-1] = '\0';
        }
    }
    return passphrase;
}

#endif // TEST_FIXTURES_H