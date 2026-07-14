#include "../catch2/catch_amalgamated.hpp"
#include "../../model/game_state.hpp"

TEST_CASE("new game state has no selection") {
    Board board(2, 2);
    GameState gameState(board);
    REQUIRE(gameState.getSelectedPosition().has_value() == false);
}

TEST_CASE("select then clearSelection roundtrip") {
    Board board(2, 2);
    GameState gameState(board);

    gameState.select(Position{0, 1});
    REQUIRE(gameState.getSelectedPosition().value() == Position{0, 1});

    gameState.clearSelection();
    REQUIRE(gameState.getSelectedPosition().has_value() == false);
}

TEST_CASE("advanceTime accumulates across multiple calls") {
    Board board(2, 2);
    GameState gameState(board);

    gameState.advanceTime(100);
    gameState.advanceTime(250);

    REQUIRE(gameState.getCurrentTime() == 350);
}

TEST_CASE("a pending move is not completed before its time arrives") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.advanceTime(500);
    REQUIRE(gameState.extractCompletedMoves().empty());
}

TEST_CASE("a pending move completes once enough time has passed") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.advanceTime(1000);
    auto completed = gameState.extractCompletedMoves();
    REQUIRE(completed.size() == 1);
    REQUIRE(completed[0].from == Position{0, 0});
    REQUIRE(completed[0].to == Position{1, 1});
}

TEST_CASE("extractCompletedMoves does not return the same move twice") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.advanceTime(1000);
    gameState.extractCompletedMoves();
    REQUIRE(gameState.extractCompletedMoves().empty());
}

TEST_CASE("requesting a new move while one is already pending is ignored") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.requestMove(Position{0, 0}, Position{0, 1});
    gameState.advanceTime(1000);
    auto completed = gameState.extractCompletedMoves();
    REQUIRE(completed.size() == 1);
    REQUIRE(completed[0].to == Position{1, 1});
}

TEST_CASE("cancelPendingMove removes a move without completing it") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.cancelPendingMove(Position{0, 0});
    gameState.advanceTime(1000);
    REQUIRE(gameState.extractCompletedMoves().empty());
}

TEST_CASE("move duration is proportional to distance travelled") {
    Board board(5, 5);
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{0, 3});
    gameState.advanceTime(2999);
    REQUIRE(gameState.extractCompletedMoves().empty());
    gameState.advanceTime(1);
    REQUIRE(gameState.extractCompletedMoves().size() == 1);
}

TEST_CASE("diagonal move duration uses the larger of row and column distance") {
    Board board(5, 5);
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{2, 2});
    gameState.advanceTime(1999);
    REQUIRE(gameState.extractCompletedMoves().empty());
    gameState.advanceTime(1);
    REQUIRE(gameState.extractCompletedMoves().size() == 1);
}

TEST_CASE("new game state is not over") {
    Board board(2, 2);
    GameState gameState(board);
    REQUIRE(gameState.isGameOver() == false);
}

TEST_CASE("endGame marks the game as over") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.endGame();
    REQUIRE(gameState.isGameOver() == true);
}

TEST_CASE("startJump makes hasActiveJumpAt true immediately") {
    Board board(3, 3);
    GameState gameState(board);
    gameState.startJump(Position{1, 1});
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == true);
}

TEST_CASE("hasActiveJumpAt is false for a different position") {
    Board board(3, 3);
    GameState gameState(board);
    gameState.startJump(Position{1, 1});
    REQUIRE(gameState.hasActiveJumpAt(Position{0, 0}) == false);
}

TEST_CASE("a jump is still active exactly at its expiry time") {
    Board board(3, 3);
    GameState gameState(board);
    gameState.startJump(Position{1, 1});
    gameState.advanceTime(1000);
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == true);
}

TEST_CASE("a jump is no longer active after its duration has fully elapsed") {
    Board board(3, 3);
    GameState gameState(board);
    gameState.startJump(Position{1, 1});
    gameState.advanceTime(1001);
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == false);
}

TEST_CASE("clearJumpAt removes an active jump before it expires") {
    Board board(3, 3);
    GameState gameState(board);
    gameState.startJump(Position{1, 1});
    gameState.clearJumpAt(Position{1, 1});
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == false);
}

TEST_CASE("clearJumpAt on a position with no jump does nothing") {
    Board board(3, 3);
    GameState gameState(board);
    gameState.clearJumpAt(Position{1, 1});
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == false);
}
