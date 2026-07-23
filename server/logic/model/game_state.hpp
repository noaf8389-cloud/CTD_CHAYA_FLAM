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

    static const inline long long LONG_REST_DURATION_MS = 3000;
    static const inline long long SHORT_REST_DURATION_MS = 1000;

    void startJump(const Position& position) {
        jumps_.push_back(Jump{position, currentTime_ + JUMP_DURATION_MS});}

    // Checks whether a piece is currently airborne (mid-jump) at the given position.
    bool hasActiveJumpAt(const Position& position) const;
    // Removes the jump record at the given position, if one exists.
    void clearJumpAt(const Position& position);

    explicit GameState(Board board) : board_(std::move(board)) {}

    Board& getBoard() { return board_; }
    const Board& getBoard() const { return board_; }

    std::optional<Position> getSelectedPosition() const { return selectedPosition_; }
    void select(const Position& position) { selectedPosition_ = position; }
    void clearSelection() { selectedPosition_ = std::nullopt; }

    long long getCurrentTime() const { return currentTime_; }
    void advanceTime(long long ms) { currentTime_ += ms; }

    // Checks whether a piece currently has a move in progress from the given position.
    bool hasPendingMove(const Position& from) const;
    // Returns the in-progress move starting from the given position, if any.
    std::optional<Motion> getPendingMove(const Position& from) const;

    // Queues a move from one position to another, to complete after a distance-based delay.
    void requestMove(const Position& from, const Position& to);
    // Cancels the in-progress move starting from the given position, if any.
    void cancelPendingMove(const Position& from);
    // Removes and returns all queued moves whose completion time has arrived.
    std::vector<Motion> extractCompletedMoves();

    // Removes and returns the positions of all rests whose duration has elapsed.
    std::vector<Position> extractExpiredRests();

    bool isGameOver() const { return gameOver_; }
    void endGame() { gameOver_ = true; }

    // Checks whether a piece at the given position is still in its post-move rest period.
    bool isResting(const Position& position) const;
    void startLongRest(const Position& position) { rests_.push_back(Rest{position, currentTime_ + LONG_REST_DURATION_MS}); }
    void startShortRest(const Position& position) { rests_.push_back(Rest{position, currentTime_ + SHORT_REST_DURATION_MS}); }
    // Removes and returns the positions of all jumps whose airborne time has elapsed.
    std::vector<Position> extractExpiredJumps();

private:
    Board board_;
    std::optional<Position> selectedPosition_;
    long long currentTime_ = 0;
    std::vector<Motion> pendingMoves_;
    char playerColor_ = DEFAULT_PLAYER_COLOR;
    bool gameOver_ = false;
    std::vector<Jump> jumps_;
    std::vector<Rest> rests_;
};
