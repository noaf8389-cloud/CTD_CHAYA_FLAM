#include "game_session.hpp"

#include <vector>

#include "bus/game_events.hpp"
#include "logic/real_time/real_time_arbiter.hpp"

void GameSession::handleClick(int row, int col) {
    std::lock_guard lock(mutex_);

    std::optional<Motion> started = Controller::handleClick(row, col, gameState_);
    if (!started.has_value()) {
        return;
    }

    std::string token = gameState_.getBoard().getCell(started->from.row, started->from.col);
    long long now = gameState_.getCurrentTime();
    long long duration = started->completionTime - now;

    bus_.publish(MoveStartedEvent{started->from, started->to, token, duration, now});
}

void GameSession::handleJump(int row, int col) {
    std::lock_guard lock(mutex_);

    std::optional<Position> started = Controller::handleJump(row, col, gameState_);
    if (!started.has_value()) {
        return;
    }

    std::string token = gameState_.getBoard().getCell(started->row, started->col);
    long long now = gameState_.getCurrentTime();

    bus_.publish(JumpStartedEvent{*started, token, GameState::JUMP_DURATION_MS, now});
}

void GameSession::update(long long deltaMs) {
    std::lock_guard lock(mutex_);

    gameState_.advanceTime(deltaMs);

    for (const Position& position : RealTimeArbiter::applyExpiredJumps(gameState_)) {
        std::string token = gameState_.getBoard().getCell(position.row, position.col);
        long long now = gameState_.getCurrentTime();
        bus_.publish(JumpLandedEvent{position, token, now});
    }

    for (const CompletedMoveResult& result : RealTimeArbiter::applyCompletedMoves(gameState_)) {
        long long now = gameState_.getCurrentTime();

        bus_.publish(MoveMadeEvent{result.from, result.to, result.movedToken, now});

        if (result.captured) {
            bus_.publish(PieceCapturedEvent{result.to, result.survivingToken, result.capturedToken, now});
        }

        if (result.gameEnded) {
            bus_.publish(GameOverEvent{result.survivingToken[0], now});
        }
    }

    for (const Position& position : gameState_.extractExpiredRests()) {
        std::string token = gameState_.getBoard().getCell(position.row, position.col);
        long long now = gameState_.getCurrentTime();
        bus_.publish(RestEndedEvent{position, token, now});
    }
}

GameStartedEvent GameSession::buildGameStartedEvent() const {
    std::lock_guard lock(mutex_);

    const Board& board = gameState_.getBoard();
    std::vector<std::string> cells;
    cells.reserve(board.getRowCount() * board.getColCount());
    for (int row = 0; row < board.getRowCount(); ++row) {
        for (int col = 0; col < board.getColCount(); ++col) {
            cells.push_back(board.getCell(row, col));
        }
    }
    return GameStartedEvent{board.getRowCount(), board.getColCount(), cells, gameState_.getCurrentTime()};
}
