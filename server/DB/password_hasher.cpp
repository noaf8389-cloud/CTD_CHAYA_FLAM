#include "password_hasher.hpp"
#include "../third_party/picosha2/picosha2.h"

#include <random>
#include <sstream>
#include <iomanip>

std::string PasswordHasher::generateSalt() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dist;

    std::ostringstream oss;
    oss << std::hex << std::setw(kSaltHexDigits) << std::setfill('0') << dist(gen);
    return oss.str();
}

std::string PasswordHasher::hash(const std::string& password, const std::string& salt) {
    return picosha2::hash256_hex_string(salt + password);
}
