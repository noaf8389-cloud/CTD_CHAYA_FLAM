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

private:
    Board board_;
    std::optional<Position> selectedPosition_;
    long long currentTime_ = 0;
    std::vector<Motion> pendingMoves_;
    char playerColor_ = DEFAULT_PLAYER_COLOR;
};
