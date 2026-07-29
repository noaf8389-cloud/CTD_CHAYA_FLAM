// room_message.hpp
#pragma once
#include <optional>
#include <string>

struct JoinRoomRequest {
    std::string roomCode;
};

// True if rawMessage is a "createRoom" command.
bool parseCreateRoomRequest(const std::string& rawMessage);

// Parses rawMessage as a "joinRoom" command and extracts the room code.
std::optional<JoinRoomRequest> parseJoinRoomRequest(const std::string& rawMessage);

// True if rawMessage is a "findMatch" command.
bool parseFindMatchRequest(const std::string& rawMessage);
