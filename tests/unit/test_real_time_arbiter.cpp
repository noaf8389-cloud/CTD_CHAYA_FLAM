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

TEST_CASE("applying a move whose source is already empty does not erase the destination") {
    Board board(2, 2);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);
    REQUIRE(gameState.getBoard().getCell(1, 1) == "wK");
}

TEST_CASE("applyCompletedMoves ends the game when a king is captured") {
    Board board(2, 2);
    board.setCell(0, 0, "wR");
    board.setCell(0, 1, "bK");
    GameState gameState(board);

    gameState.requestMove(Position{0, 0}, Position{0, 1});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.isGameOver() == true);
    REQUIRE(gameState.getBoard().getCell(0, 1) == "wR");
}

TEST_CASE("applyCompletedMoves does not end the game when capturing a non-king piece") {
    Board board(2, 2);
    board.setCell(0, 0, "wR");
    board.setCell(0, 1, "bQ");
    GameState gameState(board);

    gameState.requestMove(Position{0, 0}, Position{0, 1});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.isGameOver() == false);
}

TEST_CASE("applyCompletedMoves promotes a white pawn reaching the last row") {
    Board board(4, 4);
    board.setCell(1, 0, "wP");
    GameState gameState(board);

    gameState.requestMove(Position{1, 0}, Position{0, 0});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.getBoard().getCell(0, 0) == "wQ");
}

TEST_CASE("applyCompletedMoves promotes a black pawn reaching the last row") {
    Board board(4, 4);
    board.setCell(2, 0, "bP");
    GameState gameState(board);

    gameState.requestMove(Position{2, 0}, Position{3, 0});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.getBoard().getCell(3, 0) == "bQ");
}

TEST_CASE("applyCompletedMoves does not promote a pawn that has not reached the last row") {
    Board board(4, 4);
    board.setCell(2, 0, "wP");
    GameState gameState(board);

    gameState.requestMove(Position{2, 0}, Position{1, 0});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.getBoard().getCell(1, 0) == "wP");
}

TEST_CASE("applyCompletedMoves does not promote a non-pawn piece reaching the last row") {
    Board board(4, 4);
    board.setCell(1, 0, "wR");
    GameState gameState(board);

    gameState.requestMove(Position{1, 0}, Position{0, 0});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.getBoard().getCell(0, 0) == "wR");
}

TEST_CASE("a pawn moving two cells directly onto the promotion row still promotes to queen") {
    Board board(3, 4);
    board.setCell(2, 0, "wP");
    GameState gameState(board);

    gameState.requestMove(Position{2, 0}, Position{0, 0});
    gameState.advanceTime(2000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.getBoard().getCell(0, 0) == "wQ");
}

TEST_CASE("a pawn capturing diagonally into the promotion row promotes to queen") {
    Board board(4, 4);
    board.setCell(1, 0, "wP");
    board.setCell(0, 1, "bK");
    GameState gameState(board);

    gameState.requestMove(Position{1, 0}, Position{0, 1});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.getBoard().getCell(0, 1) == "wQ");
}
