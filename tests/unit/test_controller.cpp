#include "../catch2/catch_amalgamated.hpp"
#include "../../input/controller.hpp"

namespace {
    Board makeBoardWithPieces() {
        Board board(2, 2);
        board.setCell(0, 0, "wK");
        board.setCell(0, 1, "wQ");
        board.setCell(1, 0, "bK");
        return board;
    }
}

TEST_CASE("clicking outside the board does nothing") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(500, 500, gameState);
    REQUIRE(gameState.getSelectedPosition().has_value() == false);
}

TEST_CASE("clicking an empty cell with nothing selected is ignored") {
    Board board(2, 2);
    GameState gameState(board);
    Controller::handleClick(50, 50, gameState);
    REQUIRE(gameState.getSelectedPosition().has_value() == false);
}

TEST_CASE("clicking own piece with nothing selected selects it") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(50, 50, gameState);
    REQUIRE(gameState.getSelectedPosition().value() == Position{0, 0});
}

TEST_CASE("clicking opponent piece with nothing selected is ignored") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(50, 150, gameState);
    REQUIRE(gameState.getSelectedPosition().has_value() == false);
}

TEST_CASE("clicking the same selected cell again does nothing") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(50, 50, gameState);
    Controller::handleClick(50, 50, gameState);
    REQUIRE(gameState.getSelectedPosition().value() == Position{0, 0});
}

TEST_CASE("clicking another friendly piece switches selection") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(50, 50, gameState);
    Controller::handleClick(150, 50, gameState);
    REQUIRE(gameState.getSelectedPosition().value() == Position{0, 1});
}

TEST_CASE("clicking an empty cell while a piece is selected sends a move request and clears selection") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(50, 50, gameState);
    Controller::handleClick(150, 150, gameState);

    REQUIRE(gameState.getSelectedPosition().has_value() == false);

    gameState.advanceTime(100000);
    auto completed = gameState.extractCompletedMoves();
    REQUIRE(completed.size() == 1);
    REQUIRE(completed[0].from == Position{0, 0});
    REQUIRE(completed[0].to == Position{1, 1});
}

TEST_CASE("clicking an enemy piece while a piece is selected sends a move request (capture)") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(50, 50, gameState);
    Controller::handleClick(50, 150, gameState);

    REQUIRE(gameState.getSelectedPosition().has_value() == false);

    gameState.advanceTime(100000);
    auto completed = gameState.extractCompletedMoves();
    REQUIRE(completed.size() == 1);
    REQUIRE(completed[0].to == Position{1, 0});
}

TEST_CASE("clicking outside the board while a piece is selected keeps the selection") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(50, 50, gameState);
    Controller::handleClick(500, 500, gameState);
    REQUIRE(gameState.getSelectedPosition().value() == Position{0, 0});
}
