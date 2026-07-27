#include "../../catch2/catch_amalgamated.hpp"
#include "../../../db/password_hasher.hpp"

TEST_CASE("hashing the same password with the same salt is deterministic") {
    std::string salt = "abc123";
    REQUIRE(PasswordHasher::hash("secret", salt) == PasswordHasher::hash("secret", salt));
}

TEST_CASE("different salts produce different hashes for the same password") {
    REQUIRE(PasswordHasher::hash("secret", "salt1") != PasswordHasher::hash("secret", "salt2"));
}

TEST_CASE("different passwords with the same salt produce different hashes") {
    std::string salt = "abc123";
    REQUIRE(PasswordHasher::hash("secret1", salt) != PasswordHasher::hash("secret2", salt));
}

TEST_CASE("generateSalt produces different values on successive calls") {
    REQUIRE(PasswordHasher::generateSalt() != PasswordHasher::generateSalt());
}

TEST_CASE("hashing an empty password does not throw") {
    REQUIRE_NOTHROW(PasswordHasher::hash("", "somesalt"));
}

TEST_CASE("generateSalt returns a 16-character lowercase hex string") {
    std::string salt = PasswordHasher::generateSalt();
    REQUIRE(salt.size() == 16);
    REQUIRE(salt.find_first_not_of("0123456789abcdef") == std::string::npos);
}

TEST_CASE("hashing supports non-ASCII passwords") {
    REQUIRE_NOTHROW(PasswordHasher::hash(u8"\u05e1\u05d9\u05e1\u05de\u05d4123", "salt"));
}
