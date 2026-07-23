#pragma once

#include <functional>
#include <string>
#include <unordered_map>

class GameSession;

class CommandHandler {
public:
    /// Creates a command handler for the given game state.
    explicit CommandHandler(GameSession& gameState);

    /// Parses and executes a command received from a client.
    void handleMessage(const std::string& rawMessage);

private:
    using Handler = std::function<void(int row, int col)>;
    std::unordered_map<std::string, Handler> handlers_;
};
