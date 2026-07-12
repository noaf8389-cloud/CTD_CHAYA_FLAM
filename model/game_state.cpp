#include "game_state.hpp"

void GameState::requestMove(const Position& from, const Position& to, long long durationMs) {
    cancelPendingMove(from);
    pendingMoves_.push_back(Motion{from, to, currentTime_ + durationMs});
}

void GameState::cancelPendingMove(const Position& from) {
    for (size_t i = 0; i < pendingMoves_.size(); ++i) {
        if (pendingMoves_[i].from == from) {
            pendingMoves_.erase(pendingMoves_.begin() + i);
            return;
        }
    }
}

std::vector<Motion> GameState::extractCompletedMoves() {
    std::vector<Motion> completed;
    std::vector<Motion> stillPending;

    for (const Motion& move : pendingMoves_) {
        if (move.completionTime <= currentTime_) {
            completed.push_back(move);
        } else {
            stillPending.push_back(move);
        }
    }

    pendingMoves_ = stillPending;
    return completed;
}
