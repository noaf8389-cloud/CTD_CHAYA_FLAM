#pragma once
#include <optional>
#include <vector>
#include "Position.hpp"
#include "board.hpp"
#include "../real_time/motion.hpp"

class GameState {
public:
    static const inline char DEFAULT_PLAYER_COLOR = 'w';

    static const inline long long MS_PER_CELL = 1000;

    static const inline long long JUMP_DURATION_MS = 1000;

    void startJump(const Position& position) {
        jumps_.push_back(Jump{position, currentTime_ + JUMP_DURATION_MS});}

    bool hasActiveJumpAt(const Position& position) const;
    void clearJumpAt(const Position& position);

    explicit GameState(Board board) : board_(std::move(board)) {}

    Board& getBoard() { return board_; }
    const Board& getBoard() const { return board_; }

    std::optional<Position> getSelectedPosition() const { return selectedPosition_; }
    void select(const Position& position) { selectedPosition_ = position; }
    void clearSelection() { selectedPosition_ = std::nullopt; }

    long long getCurrentTime() const { return currentTime_; }
    void advanceTime(long long ms) { currentTime_ += ms; }

    bool hasPendingMove(const Position& from) const;

    void requestMove(const Position& from, const Position& to);
    void cancelPendingMove(const Position& from);
    std::vector<Motion> extractCompletedMoves();

    bool isGameOver() const { return gameOver_; }
    void endGame() { gameOver_ = true; }

private:
    Board board_;
    std::optional<Position> selectedPosition_;
    long long currentTime_ = 0;
    std::vector<Motion> pendingMoves_;
    char playerColor_ = DEFAULT_PLAYER_COLOR;
    bool gameOver_ = false;
    std::vector<Jump> jumps_;
};
