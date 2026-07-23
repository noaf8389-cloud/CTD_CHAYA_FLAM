#pragma once
#include <vector>
#include <string>
#include "../model/game_state.hpp"

class GameEngine {
public:
    // Executes a full script of text commands (click/jump/wait/print) against a game state.
    static void run(GameState& gameState, const std::vector<std::string>& commands);
    // Parses a "jump X Y" command string, returning the extracted coordinates on success.
    static bool tryParseJumpCommand(const std::string& command, int& outX, int& outY);

private:
    static bool tryExtractKeyword(const std::string& command, const std::string& expectedKeyword, std::istringstream& stream);
    static bool tryParseWaitCommand(const std::string& command, long long& outMs);
    static bool tryParseClickCommand(const std::string& command, int& outX, int& outY);
};
