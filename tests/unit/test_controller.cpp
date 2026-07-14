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

TEST_CASE("clicking any piece with nothing selected selects it regardless of color") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(50, 150, gameState);
    REQUIRE(gameState.getSelectedPosition().value() == Position{1, 0});
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

TEST_CASE("clicking an illegal target while a rook is selected is ignored and keeps selection") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);

    Controller::handleClick(50, 50, gameState);
    Controller::handleClick(350, 350, gameState);

    REQUIRE(gameState.getSelectedPosition().value() == Position{0, 0});

    gameState.advanceTime(100000);
    REQUIRE(gameState.extractCompletedMoves().empty());
}

TEST_CASE("clicking a legal target while a rook is selected sends a move request and clears selection") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);

    Controller::handleClick(50, 50, gameState);
    Controller::handleClick(350, 50, gameState);

    REQUIRE(gameState.getSelectedPosition().has_value() == false);

    gameState.advanceTime(100000);
    REQUIRE(gameState.extractCompletedMoves().size() == 1);
}

TEST_CASE("clicking to capture with an illegal shape is ignored") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(1, 1, "bK");
    GameState gameState(board);

    Controller::handleClick(50, 50, gameState);
    Controller::handleClick(150, 150, gameState);

    REQUIRE(gameState.getSelectedPosition().value() == Position{0, 0});

    gameState.advanceTime(100000);
    REQUIRE(gameState.extractCompletedMoves().empty());
}

TEST_CASE("clicking to capture with a legal shape sends a move request") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 3, "bK");
    GameState gameState(board);

    Controller::handleClick(50, 50, gameState);
    Controller::handleClick(350, 50, gameState);

    REQUIRE(gameState.getSelectedPosition().has_value() == false);

    gameState.advanceTime(100000);
    auto completed = gameState.extractCompletedMoves();
    REQUIRE(completed.size() == 1);
    REQUIRE(completed[0].to == Position{0, 3});
}

TEST_CASE("clicking a legal knight move through the controller sends a move request") {
    Board board(4, 4);
    board.setCell(0, 0, "wN");
    GameState gameState(board);

    Controller::handleClick(50, 50, gameState);
    Controller::handleClick(250, 150, gameState);

    REQUIRE(gameState.getSelectedPosition().has_value() == false);

    gameState.advanceTime(100000);
    REQUIRE(gameState.extractCompletedMoves().size() == 1);
}

TEST_CASE("clicking an illegal knight move through the controller is ignored") {
    Board board(4, 4);
    board.setCell(0, 0, "wN");
    GameState gameState(board);

    Controller::handleClick(50, 50, gameState);
    Controller::handleClick(150, 50, gameState);

    REQUIRE(gameState.getSelectedPosition().value() == Position{0, 0});
}

TEST_CASE("clicking after the game is over is ignored") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);
    gameState.endGame();

    Controller::handleClick(50, 50, gameState);

    REQUIRE(gameState.getSelectedPosition().has_value() == false);
}

TEST_CASE("clicking after the game is over does not start a new move even on a valid target") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);
    gameState.select(Position{0, 0});
    gameState.endGame();

    Controller::handleClick(150, 50, gameState);

    REQUIRE(gameState.getSelectedPosition().value() == Position{0, 0});
    REQUIRE(gameState.hasPendingMove(Position{0, 0}) == false);
}

TEST_CASE("jumping on an empty cell does nothing") {
    Board board(3, 3);
    GameState gameState(board);
    Controller::handleJump(150, 150, gameState);
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == false);
}

TEST_CASE("jumping on a stationary piece starts a jump") {
    Board board(3, 3);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    Controller::handleJump(150, 150, gameState);
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == true);
}

TEST_CASE("jumping on a piece that is already moving is ignored") {
    Board board(1, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{0, 3});

    Controller::handleJump(50, 50, gameState);

    REQUIRE(gameState.hasActiveJumpAt(Position{0, 0}) == false);
}

TEST_CASE("jumping outside the board does nothing") {
    Board board(3, 3);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    Controller::handleJump(500, 500, gameState);
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == false);
}

TEST_CASE("jumping after the game is over is ignored") {
    Board board(3, 3);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    gameState.endGame();
    Controller::handleJump(150, 150, gameState);
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == false);
}
