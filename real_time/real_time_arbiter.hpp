#pragma once
#include "../model/game_state.hpp"

class RealTimeArbiter {
public:
    static void applyCompletedMoves(GameState& gameState);
};
