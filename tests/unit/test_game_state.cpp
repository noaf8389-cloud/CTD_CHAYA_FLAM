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

TEST_CASE("default player color is white") {
    Board board(2, 2);
    GameState gameState(board);
    REQUIRE(gameState.getPlayerColor() == 'w');
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

    gameState.requestMove(Position{0, 0}, Position{1, 1}, 1000);
    gameState.advanceTime(500);

    REQUIRE(gameState.extractCompletedMoves().empty());
}

TEST_CASE("a pending move completes once enough time has passed") {
    Board board(2, 2);
    GameState gameState(board);

    gameState.requestMove(Position{0, 0}, Position{1, 1}, 1000);
    gameState.advanceTime(1000);

    auto completed = gameState.extractCompletedMoves();
    REQUIRE(completed.size() == 1);
    REQUIRE(completed[0].from == Position{0, 0});
    REQUIRE(completed[0].to == Position{1, 1});
}

TEST_CASE("extractCompletedMoves does not return the same move twice") {
    Board board(2, 2);
    GameState gameState(board);

    gameState.requestMove(Position{0, 0}, Position{1, 1}, 1000);
    gameState.advanceTime(1000);
    gameState.extractCompletedMoves();

    REQUIRE(gameState.extractCompletedMoves().empty());
}

TEST_CASE("requesting a new move from the same source cancels the previous one") {
    Board board(2, 2);
    GameState gameState(board);

    gameState.requestMove(Position{0, 0}, Position{1, 1}, 1000);
    gameState.requestMove(Position{0, 0}, Position{0, 1}, 1000);
    gameState.advanceTime(1000);

    auto completed = gameState.extractCompletedMoves();
    REQUIRE(completed.size() == 1);
    REQUIRE(completed[0].to == Position{0, 1});
}

TEST_CASE("cancelPendingMove removes a move without completing it") {
    Board board(2, 2);
    GameState gameState(board);

    gameState.requestMove(Position{0, 0}, Position{1, 1}, 1000);
    gameState.cancelPendingMove(Position{0, 0});
    gameState.advanceTime(1000);

    REQUIRE(gameState.extractCompletedMoves().empty());
}

TEST_CASE("only matured moves are extracted while others remain pending") {
    Board board(2, 2);
    GameState gameState(board);

    gameState.requestMove(Position{0, 0}, Position{0, 1}, 100);
    gameState.requestMove(Position{1, 0}, Position{1, 1}, 1000);
    gameState.advanceTime(100);

    auto completed = gameState.extractCompletedMoves();
    REQUIRE(completed.size() == 1);
    REQUIRE(completed[0].from == Position{0, 0});
}
