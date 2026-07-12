#pragma once
#include <vector>
#include <string>
#include "../model/game_state.hpp"

class GameEngine {
public:
    static void run(GameState& gameState, const std::vector<std::string>& commands);

private:
    static bool tryExtractKeyword(const std::string& command, const std::string& expectedKeyword, std::istringstream& stream);
    static bool tryParseWaitCommand(const std::string& command, long long& outMs);
    static bool tryParseClickCommand(const std::string& command, int& outX, int& outY);
};
