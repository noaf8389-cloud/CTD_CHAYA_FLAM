#include "client_board_state.hpp"

#include "bus/event_bus.hpp"

ClientBoardState::ClientBoardState(EventBus& bus) {
    bus.subscribe<GameStartedEvent>([this](const GameStartedEvent& e) { onGameStarted(e); });
    bus.subscribe<MoveMadeEvent>([this](const MoveMadeEvent& e) { onMoveMade(e); });
    bus.subscribe<PieceCapturedEvent>([this](const PieceCapturedEvent& e) { onPieceCaptured(e); });
    bus.subscribe<GameOverEvent>([this](const GameOverEvent& e) { onGameOver(e); });
}

Board ClientBoardState::board() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return board_;
}

bool ClientBoardState::is_game_over() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return game_over_;
}

void ClientBoardState::onGameStarted(const GameStartedEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    Board board(event.row_count, event.col_count);
    for (int row = 0; row < event.row_count; ++row) {
        for (int col = 0; col < event.col_count; ++col) {
            board.setCell(row, col, event.cells[row * event.col_count + col]);
        }
    }
    board_ = board;
}

void ClientBoardState::onMoveMade(const MoveMadeEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    board_.setCell(event.from.row, event.from.col, Board::EMPTY_CELL);
    board_.setCell(event.to.row, event.to.col, event.piece_token);
}

void ClientBoardState::onPieceCaptured(const PieceCapturedEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    board_.setCell(event.at.row, event.at.col, event.capturing_piece_token);
}

void ClientBoardState::onGameOver(const GameOverEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    game_over_ = true;
}
