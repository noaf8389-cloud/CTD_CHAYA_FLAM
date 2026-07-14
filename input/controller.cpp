// input/controller.cpp
#include "controller.hpp"
#include "board_mapper.hpp"
#include "../rules/piece_rules.hpp"

void Controller::handleClick(int x, int y, GameState& gameState) {
    if (gameState.isGameOver()) {
        return;
    }

    const Board& board = gameState.getBoard();
    std::optional<Position> clicked = BoardMapper::toPosition(x, y, board.getRowCount(), board.getColCount());
    if (!clicked.has_value()) {
        return;
    }

    std::optional<Position> selected = gameState.getSelectedPosition();

    if (!selected.has_value()) {
        handleClickWithNoSelection(clicked.value(), gameState);
        return;
    }

    if (clicked.value() == selected.value()) {
        return;
    }

    handleClickWithSelection(clicked.value(), selected.value(), gameState);
}

void Controller::handleClickWithSelection(const Position& clicked, const Position& selected, GameState& gameState) {
    const Board& board = gameState.getBoard();
    std::string selectedToken = board.getCell(selected.row, selected.col);
    std::string clickedToken = board.getCell(clicked.row, clicked.col);

    if (PieceRules::isSameColor(selectedToken, clickedToken)) {
        gameState.select(clicked);
        return;
    }

    if (!RulesEngine::isLegalMove(selected, clicked, board)) {
        return;
    }

    gameState.requestMove(selected, clicked);
    gameState.clearSelection();
}

void Controller::handleClickWithNoSelection(const Position& clicked, GameState& gameState) {
    const Board& board = gameState.getBoard();
    std::string token = board.getCell(clicked.row, clicked.col);

    if (token == Board::EMPTY_CELL) {
        return;
    }

    gameState.select(clicked);
}

void Controller::handleJump(int x, int y, GameState& gameState) {
    if (gameState.isGameOver()) {
        return;
    }

    const Board& board = gameState.getBoard();
    std::optional<Position> position = BoardMapper::toPosition(x, y, board.getRowCount(), board.getColCount());
    if (!position.has_value()) {
        return;
    }

    std::string token = board.getCell(position->row, position->col);
    if (token == Board::EMPTY_CELL) {
        return;
    }

    if (gameState.hasPendingMove(position.value())) {
        return;
    }

    gameState.startJump(position.value());
}
