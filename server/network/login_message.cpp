#include "login_message.hpp"
#include "../third_party/nlohmann/json.hpp"

using json = nlohmann::json;

std::optional<LoginRequest> parseLoginRequest(const std::string& rawMessage) {
    json parsed;
    try {
        parsed = json::parse(rawMessage);
    } catch (const json::parse_error&) {
        return std::nullopt;
    }

    try {
        if (parsed.value("command", "") != "login") {
            return std::nullopt;
        }

        std::string username = parsed.value("username", "");
        std::string password = parsed.value("password", "");
        if (username.empty() || password.empty()) {
            return std::nullopt;
        }
        return LoginRequest{username, password};
    } catch (const json::type_error&) {
        return std::nullopt;
    }
}
