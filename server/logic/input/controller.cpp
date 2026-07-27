#include "controller.hpp"
#include "board_mapper.hpp"
#include "../rules/piece_rules.hpp"

std::optional<Motion> Controller::handleClick(int row, int col, GameState& gameState, char actingColor) {
    if (gameState.isGameOver()) {
        return std::nullopt;
    }

    const Board& board = gameState.getBoard();
    std::optional<Position> clicked = BoardMapper::toPosition(row, col, board.getRowCount(), board.getColCount());
    if (!clicked.has_value()) {
        return std::nullopt;
    }

    std::optional<Position> selected = gameState.getSelectedPosition(actingColor);

    if (!selected.has_value()) {
        handleClickWithNoSelection(clicked.value(), gameState, actingColor);
        return std::nullopt;
    }

    if (clicked.value() == selected.value()) {
        return std::nullopt;
    }

    return handleClickWithSelection(clicked.value(), selected.value(), gameState, actingColor);
}

std::optional<Motion> Controller::handleClickWithSelection(const Position& clicked, const Position& selected, GameState& gameState, char actingColor) {
    const Board& board = gameState.getBoard();
    std::string selectedToken = board.getCell(selected.row, selected.col);
    std::string clickedToken = board.getCell(clicked.row, clicked.col);

    if (PieceRules::isSameColor(selectedToken, clickedToken)) {
        gameState.select(actingColor, clicked);
        return std::nullopt;
    }

    if (gameState.isResting(selected)) {
        return std::nullopt;
    }

    if (!RulesEngine::isLegalMove(selected, clicked, board)) {
        return std::nullopt;
    }

    gameState.requestMove(selected, clicked);
    gameState.clearSelection(actingColor);
    return gameState.getPendingMove(selected);
}

void Controller::handleClickWithNoSelection(const Position& clicked, GameState& gameState, char actingColor) {
    const Board& board = gameState.getBoard();
    std::string token = board.getCell(clicked.row, clicked.col);

    if (token == Board::EMPTY_CELL) {
        return;
    }

    if (!PieceRules::isColor(token, actingColor)) {
        return;
    }

    gameState.select(actingColor, clicked);}

std::optional<Position> Controller::handleJump(int row, int col, GameState& gameState, char actingColor) {
    if (gameState.isGameOver()) {
        return std::nullopt;
    }

    const Board& board = gameState.getBoard();
    std::optional<Position> position = BoardMapper::toPosition(row, col, board.getRowCount(), board.getColCount());
    if (!position.has_value()) {
        return std::nullopt;
    }

    std::string token = board.getCell(position->row, position->col);
    if (token == Board::EMPTY_CELL) {
        return std::nullopt;
    }


    if (!PieceRules::isColor(token, actingColor)) {
        return std::nullopt;
    }

    if (gameState.hasPendingMove(position.value()) || gameState.isResting(position.value())) {
        return std::nullopt;
    }

    gameState.startJump(position.value());
    return position;
}
