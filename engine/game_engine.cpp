#include "game_engine.hpp"
#include <sstream>
#include <iostream>
#include "../input/controller.hpp"
#include "../real_time/real_time_arbiter.hpp"
#include "../io/board_print.hpp"

bool GameEngine::tryExtractKeyword(const std::string& command, const std::string& expectedKeyword, std::istringstream& stream) {
    stream.str(command);
    std::string keyword;
    stream >> keyword;
    return keyword == expectedKeyword;
}

bool GameEngine::tryParseWaitCommand(const std::string& command, long long& outMs) {
    std::istringstream iss;
    if (!tryExtractKeyword(command, "wait", iss)) {
        return false;
    }
    return static_cast<bool>(iss >> outMs);
}

bool GameEngine::tryParseClickCommand(const std::string& command, int& outX, int& outY) {
    std::istringstream iss;
    if (!tryExtractKeyword(command, "click", iss)) {
        return false;
    }
    return static_cast<bool>(iss >> outX >> outY);
}

void GameEngine::run(GameState& gameState, const std::vector<std::string>& commands) {
    for (const std::string& command : commands) {
        long long waitMs;
        if (tryParseWaitCommand(command, waitMs)) {
            gameState.advanceTime(waitMs);
            RealTimeArbiter::applyCompletedMoves(gameState);
            continue;
        }

        int x, y;
        if (tryParseClickCommand(command, x, y)) {
            Controller::handleClick(x, y, gameState);
            continue;
        }

        if (command == "print board") {
            BoardPrinter::print(gameState.getBoard(), std::cout);
        }
    }
}
