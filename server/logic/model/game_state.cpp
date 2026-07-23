#include "game_state.hpp"

bool GameState::hasActiveJumpAt(const Position& position) const {
    for (const Jump& jump : jumps_) {
        if (jump.position == position && jump.endTime >= currentTime_) {
            return true;
        }
    }
    return false;
}

void GameState::clearJumpAt(const Position& position) {
    for (size_t i = 0; i < jumps_.size(); ++i) {
        if (jumps_[i].position == position) {
            jumps_.erase(jumps_.begin() + i);
            return;
        }
    }
}

void GameState::requestMove(const Position& from, const Position& to) {
    if (!pendingMoves_.empty()) {
        return;
    }
    int rowDiff = std::abs(from.row - to.row);
    int colDiff = std::abs(from.col - to.col);
    long long distance = std::max(rowDiff, colDiff);
    pendingMoves_.push_back(Motion{from, to, currentTime_ + distance * MS_PER_CELL});
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

std::vector<Position> GameState::extractExpiredJumps() {
    std::vector<Position> expired;
    std::vector<Jump> stillActive;

    for (const Jump& jump : jumps_) {
        if (jump.endTime < currentTime_) {
            expired.push_back(jump.position);
        } else {
            stillActive.push_back(jump);
        }
    }

    jumps_ = stillActive;
    return expired;
}

std::vector<Position> GameState::extractExpiredRests() {
    std::vector<Position> expired;
    std::vector<Rest> stillResting;

    for (const Rest& rest : rests_) {
        if (rest.endTime < currentTime_) {
            expired.push_back(rest.position);
        } else {
            stillResting.push_back(rest);
        }
    }

    rests_ = stillResting;
    return expired;
}

bool GameState::hasPendingMove(const Position& from) const {
    for (const Motion& motion : pendingMoves_) {
        if (motion.from == from) {
            return true;
        }
    }
    return false;
}

std::optional<Motion> GameState::getPendingMove(const Position& from) const {
    for (const Motion& motion : pendingMoves_) {
        if (motion.from == from) {
            return motion;
        }
    }
    return std::nullopt;
}

bool GameState::isResting(const Position& position) const {
    for (const Rest& rest : rests_) {
        if (rest.position == position && rest.endTime >= currentTime_) {
            return true;
        }
    }
    return false;
}
