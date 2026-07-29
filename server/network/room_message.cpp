// room_message.cpp
#include "room_message.hpp"
#include "../third_party/nlohmann/json.hpp"

using json = nlohmann::json;

namespace {
    std::optional<std::string> commandOf(const std::string& rawMessage) {
        json parsed;
        try { parsed = json::parse(rawMessage); }
        catch (const json::parse_error&) { return std::nullopt; }
        try { return parsed.value("command", ""); }
        catch (const json::type_error&) { return std::nullopt; }
    }
}

bool parseCreateRoomRequest(const std::string& rawMessage) {
    auto command = commandOf(rawMessage);
    return command.has_value() && *command == "createRoom";
}

std::optional<JoinRoomRequest> parseJoinRoomRequest(const std::string& rawMessage) {
    json parsed;
    try { parsed = json::parse(rawMessage); }
    catch (const json::parse_error&) { return std::nullopt; }

    try {
        if (parsed.value("command", "") != "joinRoom") return std::nullopt;
        std::string roomCode = parsed.value("roomCode", "");
        if (roomCode.empty()) return std::nullopt;
        return JoinRoomRequest{roomCode};
    } catch (const json::type_error&) { return std::nullopt; }
}

bool parseFindMatchRequest(const std::string& rawMessage) {
    auto command = commandOf(rawMessage);
    return command.has_value() && *command == "findMatch";
}
