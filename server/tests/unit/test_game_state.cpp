#include "../catch2/catch_amalgamated.hpp"
#include "../../logic/model/game_state.hpp"

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

TEST_CASE("getPendingMove returns nullopt when nothing is pending") {
    Board board(2, 2);
    GameState gameState(board);
    REQUIRE(gameState.getPendingMove(Position{0, 0}).has_value() == false);
}

TEST_CASE("getPendingMove returns the motion details while pending") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});

    auto motion = gameState.getPendingMove(Position{0, 0});
    REQUIRE(motion.has_value());
    REQUIRE(motion->from == Position{0, 0});
    REQUIRE(motion->to == Position{1, 1});
}

TEST_CASE("getPendingMove returns nullopt after the move completes") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});
    gameState.advanceTime(1000);
    gameState.extractCompletedMoves();

    REQUIRE(gameState.getPendingMove(Position{0, 0}).has_value() == false);
}

TEST_CASE("getPendingMove for a different position than the pending move is nullopt") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{1, 1});

    REQUIRE(gameState.getPendingMove(Position{1, 0}).has_value() == false);
}

TEST_CASE("a position is not resting by default") {
    Board board(2, 2);
    GameState gameState(board);
    REQUIRE(gameState.isResting(Position{0, 0}) == false);
}

TEST_CASE("startLongRest makes isResting true immediately") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startLongRest(Position{0, 0});
    REQUIRE(gameState.isResting(Position{0, 0}) == true);
}

TEST_CASE("startShortRest makes isResting true immediately") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startShortRest(Position{0, 0});
    REQUIRE(gameState.isResting(Position{0, 0}) == true);
}

TEST_CASE("isResting is false for a different position than the one resting") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startLongRest(Position{0, 0});
    REQUIRE(gameState.isResting(Position{1, 1}) == false);
}

TEST_CASE("a long rest ends after LONG_REST_DURATION_MS") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startLongRest(Position{0, 0});
    gameState.advanceTime(GameState::LONG_REST_DURATION_MS + 1);
    REQUIRE(gameState.isResting(Position{0, 0}) == false);
}

TEST_CASE("a short rest ends after SHORT_REST_DURATION_MS") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startShortRest(Position{0, 0});
    gameState.advanceTime(GameState::SHORT_REST_DURATION_MS + 1);
    REQUIRE(gameState.isResting(Position{0, 0}) == false);
}

TEST_CASE("a short rest ends before a long rest would, confirming the durations actually differ") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startLongRest(Position{0, 0});
    gameState.startShortRest(Position{1, 1});
    gameState.advanceTime(GameState::SHORT_REST_DURATION_MS + 1);

    REQUIRE(gameState.isResting(Position{1, 1}) == false);
    REQUIRE(gameState.isResting(Position{0, 0}) == true);
}

TEST_CASE("extractExpiredJumps returns nothing when there are no jumps") {
    Board board(2, 2);
    GameState gameState(board);
    REQUIRE(gameState.extractExpiredJumps().empty());
}

TEST_CASE("extractExpiredJumps returns nothing while a jump is still active") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startJump(Position{0, 0});
    gameState.advanceTime(GameState::JUMP_DURATION_MS);
    REQUIRE(gameState.extractExpiredJumps().empty());
}

TEST_CASE("extractExpiredJumps returns the position once the jump has truly expired") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startJump(Position{0, 0});
    gameState.advanceTime(GameState::JUMP_DURATION_MS + 1);

    auto expired = gameState.extractExpiredJumps();
    REQUIRE(expired.size() == 1);
    REQUIRE(expired[0] == Position{0, 0});
}

TEST_CASE("extractExpiredJumps does not return the same jump twice") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startJump(Position{0, 0});
    gameState.advanceTime(GameState::JUMP_DURATION_MS + 1);
    gameState.extractExpiredJumps();

    REQUIRE(gameState.extractExpiredJumps().empty());
}

TEST_CASE("extractExpiredRests returns nothing when there are no rests") {
    Board board(2, 2);
    GameState gameState(board);
    REQUIRE(gameState.extractExpiredRests().empty());
}

TEST_CASE("extractExpiredRests returns nothing while a long rest is still active") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startLongRest(Position{0, 0});
    gameState.advanceTime(GameState::LONG_REST_DURATION_MS);
    REQUIRE(gameState.extractExpiredRests().empty());
}

TEST_CASE("extractExpiredRests returns nothing while a short rest is still active") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startShortRest(Position{0, 0});
    gameState.advanceTime(GameState::SHORT_REST_DURATION_MS);
    REQUIRE(gameState.extractExpiredRests().empty());
}

TEST_CASE("extractExpiredRests returns the position once a long rest has truly expired") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startLongRest(Position{0, 0});
    gameState.advanceTime(GameState::LONG_REST_DURATION_MS + 1);

    auto expired = gameState.extractExpiredRests();
    REQUIRE(expired.size() == 1);
    REQUIRE(expired[0] == Position{0, 0});
}

TEST_CASE("extractExpiredRests returns the position once a short rest has truly expired") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startShortRest(Position{1, 1});
    gameState.advanceTime(GameState::SHORT_REST_DURATION_MS + 1);

    auto expired = gameState.extractExpiredRests();
    REQUIRE(expired.size() == 1);
    REQUIRE(expired[0] == Position{1, 1});
}

TEST_CASE("extractExpiredRests does not return the same rest twice") {
    Board board(2, 2);
    GameState gameState(board);
    gameState.startLongRest(Position{0, 0});
    gameState.advanceTime(GameState::LONG_REST_DURATION_MS + 1);
    gameState.extractExpiredRests();

    REQUIRE(gameState.extractExpiredRests().empty());
}

TEST_CASE("extractExpiredRests leaves a still-active rest tracked while returning only the expired one") {
    Board board(3, 3);
    GameState gameState(board);
    gameState.startShortRest(Position{0, 0});
    gameState.startLongRest(Position{1, 1});
    gameState.advanceTime(GameState::SHORT_REST_DURATION_MS + 1);

    auto expired = gameState.extractExpiredRests();
    REQUIRE(expired.size() == 1);
    REQUIRE(expired[0] == Position{0, 0});
    REQUIRE(gameState.isResting(Position{1, 1}) == true);
}

TEST_CASE("extractExpiredRests returns multiple positions that expired in the same tick") {
    Board board(3, 3);
    GameState gameState(board);
    gameState.startShortRest(Position{0, 0});
    gameState.startShortRest(Position{2, 2});
    gameState.advanceTime(GameState::SHORT_REST_DURATION_MS + 1);

    REQUIRE(gameState.extractExpiredRests().size() == 2);
}
