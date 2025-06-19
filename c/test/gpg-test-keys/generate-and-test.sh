#!/bin/bash
set -e

echo "=== GPG Test Key Generation for OpenPGP C Wrapper ==="
echo "Generating RSA 2048-bit keypair without passphrase..."

# Setup temporary GPG home to avoid polluting main keyring
export GNUPGHOME=$(mktemp -d)
trap "rm -rf $GNUPGHOME" EXIT

echo "Using temporary GPG home: $GNUPGHOME"

# Create batch configuration for automated key generation
cat > gpg-batch-config.txt <<EOF
%echo Generating RSA 2048 test key without passphrase
Key-Type: RSA
Key-Length: 2048
Subkey-Type: RSA
Subkey-Length: 2048
Name-Real: Test User
Name-Email: test@example.com
Expire-Date: 0
%no-protection
%commit
%echo done
EOF

# Generate the keypair
echo -e "\n1. Generating keypair..."
gpg --batch --generate-key gpg-batch-config.txt

# Get the key ID/fingerprint
KEY_ID=$(gpg --list-secret-keys --with-colons | grep '^fpr' | cut -d: -f10)
echo "Generated key ID: $KEY_ID"

# Export keys in ASCII armor format
echo -e "\n2. Exporting keys..."
gpg --armor --export $KEY_ID > test-public-key.asc
gpg --armor --export-secret-keys $KEY_ID > test-private-key.asc
echo "✓ Public key exported to: test-public-key.asc"
echo "✓ Private key exported to: test-private-key.asc (no passphrase)"

# Create test data
echo -e "\n3. Creating test data..."
echo "hello world" > test-message.txt
echo "Test message content: $(cat test-message.txt)"

# Test encryption with GPG
echo -e "\n4. Testing encryption..."
gpg --trust-model always --armor --encrypt --recipient $KEY_ID --output test-message.txt.asc test-message.txt
echo "✓ Created encrypted file: test-message.txt.asc"
echo "Encrypted message preview:"
head -5 test-message.txt.asc

# Test decryption with GPG (should not prompt for passphrase)
echo -e "\n5. Testing decryption..."
gpg --decrypt test-message.txt.asc > decrypted-message.txt 2>/dev/null
echo "Decrypted content: $(cat decrypted-message.txt)"

# Verify round-trip
echo -e "\n6. Verifying round-trip encryption/decryption..."
if diff -q test-message.txt decrypted-message.txt >/dev/null; then
    echo "✓ SUCCESS! Encryption/decryption working correctly with no passphrase"
else
    echo "✗ FAILED! Messages don't match"
    exit 1
fi

# Display key information
echo -e "\n7. Key information:"
echo "Public key (first 15 lines):"
head -15 test-public-key.asc
echo -e "\nPrivate key (first 15 lines):"
head -15 test-private-key.asc

# Create C header file with the keys
echo -e "\n8. Creating C header file..."
cat > test-keys.h <<EOF
#ifndef TEST_KEYS_H
#define TEST_KEYS_H

// Generated test keys without passphrase for C wrapper testing
// Generated on: $(date)

const char *test_public_key_no_passphrase = 
EOF

# Add public key with proper C string formatting
while IFS= read -r line; do
    echo "    \"$line\\\\n\"" >> test-keys.h
done < test-public-key.asc
echo "    ;" >> test-keys.h

echo "" >> test-keys.h
echo "const char *test_private_key_no_passphrase = " >> test-keys.h

# Add private key with proper C string formatting
while IFS= read -r line; do
    echo "    \"$line\\\\n\"" >> test-keys.h
done < test-private-key.asc
echo "    ;" >> test-keys.h

echo "" >> test-keys.h
echo "#endif // TEST_KEYS_H" >> test-keys.h

echo "✓ Created C header file: test-keys.h"

echo -e "\n=== Test completed successfully ==="
echo "Generated files in $(pwd):"
echo "  - test-public-key.asc     (ASCII armored public key)"
echo "  - test-private-key.asc    (ASCII armored private key, no passphrase)"
echo "  - test-message.txt        (original message)"
echo "  - test-message.txt.asc    (encrypted message)"
echo "  - decrypted-message.txt   (decrypted message)"
echo "  - test-keys.h             (C header file with keys)"
echo ""
echo "To use in C tests:"
echo "  #include \"test-keys.h\""
echo "  openpgp_decrypt(encrypted, test_private_key_no_passphrase, NULL, NULL);"