#pragma once

#include <string>

namespace PasswordHasher {
    // Generates a new random salt, unique per account.
    std::string generateSalt();
    // Combines a password with a salt and returns the resulting SHA-256 hash, hex-encoded.
    std::string hash(const std::string& password, const std::string& salt);

    // Hex digits needed to print a 64-bit salt value in full (64 bits / 4 bits per hex digit).
    constexpr int kSaltHexDigits = 16;
}
