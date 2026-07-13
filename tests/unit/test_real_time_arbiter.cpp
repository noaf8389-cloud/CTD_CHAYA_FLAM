#include "../catch2/catch_amalgamated.hpp"
#include "../../real_time/real_time_arbiter.hpp"

TEST_CASE("applyCompletedMoves does nothing when there are no pending moves") {
    Board board(2, 2);
    board.setCell(0, 0, "wK");
    GameState gameState(board);

    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.getBoard().getCell(0, 0) == "wK");
}

TEST_CASE("applyCompletedMoves moves the piece once its time has come") {
    Board board(2, 2);
    board.setCell(0, 0, "wK");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);
    REQUIRE(gameState.getBoard().getCell(0, 0) == ".");
    REQUIRE(gameState.getBoard().getCell(1, 1) == "wK");
}

TEST_CASE("applyCompletedMoves leaves the board untouched before the move matures") {
    Board board(2, 2);
    board.setCell(0, 0, "wK");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.advanceTime(500);
    RealTimeArbiter::applyCompletedMoves(gameState);
    REQUIRE(gameState.getBoard().getCell(0, 0) == "wK");
    REQUIRE(gameState.getBoard().getCell(1, 1) == ".");
}

TEST_CASE("applyCompletedMoves handles multiple matured moves in one call") {
    Board board(2, 2);
    board.setCell(0, 0, "wK");
    board.setCell(0, 1, "bQ");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 0});
    gameState.requestMove(Position{0, 1}, Position{1, 1});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);
    REQUIRE(gameState.getBoard().getCell(1, 0) == "wK");
    REQUIRE(gameState.getBoard().getCell(1, 1) == "bQ");
}

TEST_CASE("applyCompletedMoves only moves the matured piece, leaving the pending one untouched") {
    Board board(5, 5);
    board.setCell(0, 0, "wK");
    board.setCell(0, 4, "bQ");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 0});   // מרחק 1
    gameState.requestMove(Position{0, 4}, Position{4, 4});   // מרחק 4
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);
    REQUIRE(gameState.getBoard().getCell(1, 0) == "wK");
    REQUIRE(gameState.getBoard().getCell(0, 4) == "bQ");
    REQUIRE(gameState.getBoard().getCell(4, 4) == ".");
}

TEST_CASE("applying a move whose source is already empty does not erase the destination") {
    Board board(2, 2);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);
    REQUIRE(gameState.getBoard().getCell(1, 1) == "wK");
}
