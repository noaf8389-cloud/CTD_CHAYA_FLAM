#pragma once
#include <string>
#include <vector>

#include "../model/game_state.hpp"

struct CompletedMoveResult {
    Position from;
    Position to;
    std::string movedToken;
    bool captured = false;
    std::string capturedToken;
    std::string survivingToken;
    bool gameEnded = false;
};

class RealTimeArbiter {
public:
    // Applies every move whose travel time has elapsed: updates the board, handles captures/promotion/game-over, and starts rest.
    static std::vector<CompletedMoveResult> applyCompletedMoves(GameState& gameState);
    // Ends every jump whose airborne time has elapsed and starts a short rest at its landing position.
    static std::vector<Position> applyExpiredJumps(GameState& gameState);
};
