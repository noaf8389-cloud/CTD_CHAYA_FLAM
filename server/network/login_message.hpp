#pragma once

#include <optional>
#include <string>

struct LoginRequest {
    std::string username;
    std::string password;
};

// Parses rawMessage as a "login" command and extracts the username/password.
// Returns std::nullopt if the command isn't "login", the JSON is malformed, or a field is missing/empty.
std::optional<LoginRequest> parseLoginRequest(const std::string& rawMessage);
