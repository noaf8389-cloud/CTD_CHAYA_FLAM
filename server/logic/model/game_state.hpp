#pragma once
#include <map>
#include <optional>
#include <vector>
#include "model/Position.hpp"
#include "model/board.hpp"
#include "../real_time/motion.hpp"

class GameState {
public:
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

    // Returns the position currently selected by the given color, if any.
    std::optional<Position> getSelectedPosition(char color) const {
        auto it = selectedPositions_.find(color);
        return it != selectedPositions_.end() ? std::optional<Position>(it->second) : std::nullopt;
    }

    void select(char color, const Position& position) { selectedPositions_[color] = position; }
    void clearSelection(char color) { selectedPositions_.erase(color); }

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

    // Queues a synchronized king+rook castling move; both complete at the same time.
    // Returns false (queues nothing) if either piece already has a move in progress.
    bool requestCastling(const Position& kingFrom, const Position& kingTo, const Position& rookFrom, const Position& rookTo);

    // Records that a piece has moved away from the given square (for castling eligibility).
    void markVacated(const Position& position) { vacatedSquares_.push_back(position); }
    // Checks whether a piece has ever moved away from the given square.
    bool wasEverVacated(const Position& position) const;

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
    std::map<char, Position> selectedPositions_;
    long long currentTime_ = 0;
    std::vector<Motion> pendingMoves_;
    bool gameOver_ = false;
    std::vector<Jump> jumps_;
    std::vector<Rest> rests_;
    std::vector<Position> vacatedSquares_;
};
