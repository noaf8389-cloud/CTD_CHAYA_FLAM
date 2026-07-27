#include "../../catch2/catch_amalgamated.hpp"
#include "../../../network/login_message.hpp"

TEST_CASE("parseLoginRequest extracts username and password from a valid login message") {
    auto result = parseLoginRequest(R"({"command":"login","username":"noa","password":"pw123"})");
    REQUIRE(result->username == "noa");
    REQUIRE(result->password == "pw123");
}

TEST_CASE("parseLoginRequest returns nullopt for a non-login command") {
    auto result = parseLoginRequest(R"({"command":"click","row":0,"col":0})");
    REQUIRE(result.has_value() == false);
}

TEST_CASE("parseLoginRequest returns nullopt for malformed JSON") {
    REQUIRE(parseLoginRequest("not valid json").has_value() == false);
}

TEST_CASE("parseLoginRequest returns nullopt when password is missing") {
    auto result = parseLoginRequest(R"({"command":"login","username":"noa"})");
    REQUIRE(result.has_value() == false);
}

TEST_CASE("parseLoginRequest returns nullopt when username is empty") {
    auto result = parseLoginRequest(R"({"command":"login","username":"","password":"pw123"})");
    REQUIRE(result.has_value() == false);
}

TEST_CASE("parseLoginRequest is case-sensitive about the command name") {
    auto result = parseLoginRequest(R"({"command":"Login","username":"noa","password":"pw123"})");
    REQUIRE(result.has_value() == false);
}

TEST_CASE("parseLoginRequest ignores unrelated extra fields") {
    auto result = parseLoginRequest(R"({"command":"login","username":"noa","password":"pw123","extra":true})");
    REQUIRE(result->username == "noa");
}

TEST_CASE("parseLoginRequest returns nullopt instead of throwing when username has the wrong JSON type") {
    auto result = parseLoginRequest(R"({"command":"login","username":123,"password":"pw123"})");
    REQUIRE(result.has_value() == false);
}

TEST_CASE("parseLoginRequest returns nullopt instead of throwing when command has the wrong JSON type") {
    auto result = parseLoginRequest(R"({"command":42,"username":"noa","password":"pw123"})");
    REQUIRE(result.has_value() == false);
}
