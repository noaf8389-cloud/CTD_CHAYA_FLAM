// room_dialog.hpp
#pragma once
#include <optional>
#include <string>

enum class MatchIntent { FindMatch, CreateRoom, JoinRoom };

struct MatchChoice {
    MatchIntent intent;
    std::optional<std::string> roomCode;   // set only when intent == JoinRoom
};

struct LoginPrompt {
    std::string username;
    std::string password;
    MatchChoice matchChoice;
};

// Shows a native Win32 prompt collecting username/password and how to start a game.
// Blocks until the player responds.
LoginPrompt promptLogin();
