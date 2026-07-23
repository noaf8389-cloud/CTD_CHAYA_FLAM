#include "../catch2/catch_amalgamated.hpp"
#include "../../logic/real_time/real_time_arbiter.hpp"

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
    Board board(4, 4);
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

TEST_CASE("applyCompletedMoves lets an airborne piece capture an arriving enemy") {
    Board board(3, 3);
    board.setCell(1, 0, "wK");
    board.setCell(1, 1, "bR");
    GameState gameState(board);

    gameState.startJump(Position{1, 0});
    gameState.requestMove(Position{1, 1}, Position{1, 0});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.getBoard().getCell(1, 0) == "wK");
    REQUIRE(gameState.getBoard().getCell(1, 1) == ".");
}

TEST_CASE("applyCompletedMoves ends the game when a jump captures an arriving enemy king") {
    Board board(3, 3);
    board.setCell(1, 0, "wR");
    board.setCell(1, 1, "bK");
    GameState gameState(board);

    gameState.startJump(Position{1, 0});
    gameState.requestMove(Position{1, 1}, Position{1, 0});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.isGameOver() == true);
    REQUIRE(gameState.getBoard().getCell(1, 0) == "wR");
}

TEST_CASE("applyCompletedMoves applies a normal capture once the jump has already expired") {
    Board board(1, 4);
    board.setCell(0, 0, "wK");
    board.setCell(0, 3, "bR");
    GameState gameState(board);

    gameState.startJump(Position{0, 0});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    gameState.requestMove(Position{0, 3}, Position{0, 0});
    gameState.advanceTime(3000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.getBoard().getCell(0, 0) == "bR");
}

TEST_CASE("applyCompletedMoves leaves the board unchanged when no enemy arrives during a jump") {
    Board board(3, 3);
    board.setCell(1, 1, "wK");
    GameState gameState(board);

    gameState.startJump(Position{1, 1});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.getBoard().getCell(1, 1) == "wK");
}

TEST_CASE("applyCompletedMoves does not let a jump capture a friendly arriving piece") {
    Board board(1, 3);
    board.setCell(0, 0, "wK");
    board.setCell(0, 2, "wR");
    GameState gameState(board);

    gameState.startJump(Position{0, 0});
    gameState.requestMove(Position{0, 2}, Position{0, 0});
    gameState.advanceTime(2000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.getBoard().getCell(0, 0) == "wR");
}

TEST_CASE("applyCompletedMoves starts a long rest at the destination") {
    Board board(2, 2);
    board.setCell(0, 0, "wK");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.advanceTime(1000);
    RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(gameState.isResting(Position{1, 1}) == true);
}

TEST_CASE("applyExpiredJumps does nothing when there are no jumps") {
    Board board(2, 2);
    GameState gameState(board);
    RealTimeArbiter::applyExpiredJumps(gameState);
    REQUIRE(gameState.isResting(Position{0, 0}) == false);
}

TEST_CASE("applyExpiredJumps starts a short rest once a jump naturally expires") {
    Board board(2, 2);
    board.setCell(0, 0, "wK");
    GameState gameState(board);
    gameState.startJump(Position{0, 0});
    gameState.advanceTime(GameState::JUMP_DURATION_MS + 1);

    RealTimeArbiter::applyExpiredJumps(gameState);

    REQUIRE(gameState.isResting(Position{0, 0}) == true);
}

TEST_CASE("applyExpiredJumps does not start a rest for a jump that has not expired yet") {
    Board board(2, 2);
    board.setCell(0, 0, "wK");
    GameState gameState(board);
    gameState.startJump(Position{0, 0});
    gameState.advanceTime(GameState::JUMP_DURATION_MS);

    RealTimeArbiter::applyExpiredJumps(gameState);

    REQUIRE(gameState.isResting(Position{0, 0}) == false);
}

TEST_CASE("applyExpiredJumps grants a short rest, not a long one") {
    Board board(2, 2);
    board.setCell(0, 0, "wK");
    GameState gameState(board);
    gameState.startJump(Position{0, 0});
    gameState.advanceTime(GameState::JUMP_DURATION_MS + 1);
    RealTimeArbiter::applyExpiredJumps(gameState);

    gameState.advanceTime(GameState::SHORT_REST_DURATION_MS + 1);
    REQUIRE(gameState.isResting(Position{0, 0}) == false);
}

TEST_CASE("applyCompletedMoves returns the motion and token for a simple move") {
    Board board(2, 2);
    board.setCell(0, 0, "wK");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.advanceTime(1000);

    auto results = RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(results.size() == 1);
    REQUIRE(results[0].from == Position{0, 0});
    REQUIRE(results[0].to == Position{1, 1});
    REQUIRE(results[0].movedToken == "wK");
    REQUIRE(results[0].captured == false);
    REQUIRE(results[0].gameEnded == false);
}

TEST_CASE("applyCompletedMoves reports capture details for a normal capture") {
    Board board(2, 2);
    board.setCell(0, 0, "wR");
    board.setCell(0, 1, "bQ");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{0, 1});
    gameState.advanceTime(1000);

    auto results = RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(results.size() == 1);
    REQUIRE(results[0].captured == true);
    REQUIRE(results[0].capturedToken == "bQ");
    REQUIRE(results[0].survivingToken == "wR");
    REQUIRE(results[0].gameEnded == false);
}

TEST_CASE("applyCompletedMoves reports gameEnded and the surviving color when a king is captured") {
    Board board(2, 2);
    board.setCell(0, 0, "wR");
    board.setCell(0, 1, "bK");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{0, 1});
    gameState.advanceTime(1000);

    auto results = RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(results.size() == 1);
    REQUIRE(results[0].gameEnded == true);
    REQUIRE(results[0].survivingToken == "wR");
    REQUIRE(results[0].capturedToken == "bK");
}

TEST_CASE("applyCompletedMoves reports the airborne defender as the survivor when it defeats an arriving enemy") {
    Board board(3, 3);
    board.setCell(1, 0, "wK");
    board.setCell(1, 1, "bR");
    GameState gameState(board);

    gameState.startJump(Position{1, 0});
    gameState.requestMove(Position{1, 1}, Position{1, 0});
    gameState.advanceTime(1000);

    auto results = RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(results.size() == 1);
    REQUIRE(results[0].captured == true);
    REQUIRE(results[0].survivingToken == "wK");
    REQUIRE(results[0].capturedToken == "bR");
    REQUIRE(results[0].movedToken == "bR");
}

TEST_CASE("applyCompletedMoves does not mark a move as captured when the destination was empty") {
    Board board(2, 2);
    board.setCell(0, 0, "wK");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.advanceTime(1000);

    auto results = RealTimeArbiter::applyCompletedMoves(gameState);

    REQUIRE(results.size() == 1);
    REQUIRE(results[0].captured == false);
}
