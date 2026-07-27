#include "../catch2/catch_amalgamated.hpp"
#include "../../logic/input/controller.hpp"
#include "../../logic/real_time/real_time_arbiter.hpp"

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
    Controller::handleClick(5, 5, gameState, 'w');
    REQUIRE(gameState.getSelectedPosition('w').has_value() == false);
}

TEST_CASE("clicking an empty cell with nothing selected is ignored") {
    Board board(2, 2);
    GameState gameState(board);
    Controller::handleClick(0, 0, gameState, 'w');
    REQUIRE(gameState.getSelectedPosition('w').has_value() == false);
}

TEST_CASE("clicking own piece with nothing selected selects it") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(0, 0, gameState, 'w');
    REQUIRE(gameState.getSelectedPosition('w').value() == Position{0, 0});
}

TEST_CASE("clicking an opponent's piece with nothing selected does not select it") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(1, 0, gameState, 'w');   // (1,0) is "bK"
    REQUIRE(gameState.getSelectedPosition('w').has_value() == false);
}

TEST_CASE("clicking the same selected cell again does nothing") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(0, 0, gameState, 'w');
    Controller::handleClick(0, 0, gameState, 'w');
    REQUIRE(gameState.getSelectedPosition('w').value() == Position{0, 0});
}

TEST_CASE("clicking another friendly piece switches selection") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(0, 0, gameState, 'w');
    Controller::handleClick(0, 1, gameState, 'w');
    REQUIRE(gameState.getSelectedPosition('w').value() == Position{0, 1});
}

TEST_CASE("clicking an empty cell while a piece is selected sends a move request and clears selection") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(0, 0, gameState, 'w');
    Controller::handleClick(1, 1, gameState, 'w');

    REQUIRE(gameState.getSelectedPosition('w').has_value() == false);

    gameState.advanceTime(100000);
    auto completed = gameState.extractCompletedMoves();
    REQUIRE(completed.size() == 1);
    REQUIRE(completed[0].from == Position{0, 0});
    REQUIRE(completed[0].to == Position{1, 1});
}

TEST_CASE("clicking an enemy piece while a piece is selected sends a move request (capture)") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(0, 0, gameState, 'w');
    Controller::handleClick(1, 0, gameState, 'w');

    REQUIRE(gameState.getSelectedPosition('w').has_value() == false);

    gameState.advanceTime(100000);
    auto completed = gameState.extractCompletedMoves();
    REQUIRE(completed.size() == 1);
    REQUIRE(completed[0].to == Position{1, 0});
}

TEST_CASE("clicking outside the board while a piece is selected keeps the selection") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(0, 0, gameState, 'w');
    Controller::handleClick(5, 5, gameState, 'w');
    REQUIRE(gameState.getSelectedPosition('w').value() == Position{0, 0});
}

TEST_CASE("clicking an illegal target while a rook is selected is ignored and keeps selection") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);

    Controller::handleClick(0, 0, gameState, 'w');
    Controller::handleClick(3, 3, gameState, 'w');

    REQUIRE(gameState.getSelectedPosition('w').value() == Position{0, 0});

    gameState.advanceTime(100000);
    REQUIRE(gameState.extractCompletedMoves().empty());
}

TEST_CASE("clicking a legal target while a rook is selected sends a move request and clears selection") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);

    Controller::handleClick(0, 0, gameState, 'w');
    Controller::handleClick(0, 3, gameState, 'w');

    REQUIRE(gameState.getSelectedPosition('w').has_value() == false);

    gameState.advanceTime(100000);
    REQUIRE(gameState.extractCompletedMoves().size() == 1);
}

TEST_CASE("clicking to capture with an illegal shape is ignored") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(1, 1, "bK");
    GameState gameState(board);

    Controller::handleClick(0, 0, gameState, 'w');
    Controller::handleClick(1, 1, gameState, 'w');

    REQUIRE(gameState.getSelectedPosition('w').value() == Position{0, 0});

    gameState.advanceTime(100000);
    REQUIRE(gameState.extractCompletedMoves().empty());
}

TEST_CASE("clicking to capture with a legal shape sends a move request") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 3, "bK");
    GameState gameState(board);

    Controller::handleClick(0, 0, gameState, 'w');
    Controller::handleClick(0, 3, gameState, 'w');

    REQUIRE(gameState.getSelectedPosition('w').has_value() == false);

    gameState.advanceTime(100000);
    auto completed = gameState.extractCompletedMoves();
    REQUIRE(completed.size() == 1);
    REQUIRE(completed[0].to == Position{0, 3});
}

TEST_CASE("clicking a legal knight move through the controller sends a move request") {
    Board board(4, 4);
    board.setCell(0, 0, "wN");
    GameState gameState(board);

    Controller::handleClick(0, 0, gameState, 'w');
    Controller::handleClick(1, 2, gameState, 'w');

    REQUIRE(gameState.getSelectedPosition('w').has_value() == false);

    gameState.advanceTime(100000);
    REQUIRE(gameState.extractCompletedMoves().size() == 1);
}

TEST_CASE("clicking an illegal knight move through the controller is ignored") {
    Board board(4, 4);
    board.setCell(0, 0, "wN");
    GameState gameState(board);

    Controller::handleClick(0, 0, gameState, 'w');
    Controller::handleClick(0, 1, gameState, 'w');

    REQUIRE(gameState.getSelectedPosition('w').value() == Position{0, 0});
}

TEST_CASE("clicking after the game is over is ignored") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);
    gameState.endGame();

    Controller::handleClick(0, 0, gameState, 'w');

    REQUIRE(gameState.getSelectedPosition('w').has_value() == false);
}

TEST_CASE("clicking after the game is over does not start a new move even on a valid target") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);
    gameState.select('w', Position{0, 0});
    gameState.endGame();

    Controller::handleClick(0, 1, gameState, 'w');

    REQUIRE(gameState.getSelectedPosition('w').value() == Position{0, 0});
    REQUIRE(gameState.hasPendingMove(Position{0, 0}) == false);
}

TEST_CASE("jumping on an empty cell does nothing") {
    Board board(3, 3);
    GameState gameState(board);
    Controller::handleJump(1, 1, gameState, 'w');
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == false);
}

TEST_CASE("jumping on a stationary piece starts a jump") {
    Board board(3, 3);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    Controller::handleJump(1, 1, gameState, 'w');
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == true);
}

TEST_CASE("jumping on an opponent's piece is ignored") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleJump(1, 0, gameState, 'w');   // (1,0) is "bK"
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 0}) == false);
}

TEST_CASE("jumping on a piece that is already moving is ignored") {
    Board board(1, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);
    gameState.requestMove(Position{0, 0}, Position{0, 3});

    Controller::handleJump(0, 0, gameState, 'w');

    REQUIRE(gameState.hasActiveJumpAt(Position{0, 0}) == false);
}

TEST_CASE("jumping outside the board does nothing") {
    Board board(3, 3);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    Controller::handleJump(5, 5, gameState, 'w');
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == false);
}

TEST_CASE("jumping after the game is over is ignored") {
    Board board(3, 3);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    gameState.endGame();
    Controller::handleJump(1, 1, gameState, 'w');
    REQUIRE(gameState.hasActiveJumpAt(Position{1, 1}) == false);
}

TEST_CASE("handleClick returns the queued motion for a legal move") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);

    Controller::handleClick(0, 0, gameState, 'w');
    std::optional<Motion> motion = Controller::handleClick(0, 3, gameState, 'w');

    REQUIRE(motion.has_value());
    REQUIRE(motion->from == Position{0, 0});
    REQUIRE(motion->to == Position{0, 3});
    REQUIRE(motion->completionTime == 3 * GameState::MS_PER_CELL);
}

TEST_CASE("handleClick returns nullopt when the click only selects a piece") {
    GameState gameState(makeBoardWithPieces());
    std::optional<Motion> motion = Controller::handleClick(0, 0, gameState, 'w');
    REQUIRE(motion.has_value() == false);
}

TEST_CASE("handleClick returns nullopt when the target is illegal") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);

    Controller::handleClick(0, 0, gameState, 'w');
    std::optional<Motion> motion = Controller::handleClick(1, 1, gameState, 'w');

    REQUIRE(motion.has_value() == false);
}

TEST_CASE("handleClick returns nullopt when clicking the same selected cell again") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(0, 0, gameState, 'w');
    std::optional<Motion> motion = Controller::handleClick(0, 0, gameState, 'w');
    REQUIRE(motion.has_value() == false);
}

TEST_CASE("handleClick returns nullopt when clicking outside the board") {
    GameState gameState(makeBoardWithPieces());
    std::optional<Motion> motion = Controller::handleClick(5, 5, gameState, 'w');
    REQUIRE(motion.has_value() == false);
}

TEST_CASE("handleClick returns nullopt after the game is over") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);
    gameState.select('w', Position{0, 0});
    gameState.endGame();

    std::optional<Motion> motion = Controller::handleClick(0, 1, gameState, 'w');
    REQUIRE(motion.has_value() == false);
}

TEST_CASE("two colors selecting independently do not interfere with each other") {
    GameState gameState(makeBoardWithPieces());
    Controller::handleClick(0, 0, gameState, 'w');   // white selects its king
    Controller::handleClick(1, 0, gameState, 'b');   // black selects its own king

    REQUIRE(gameState.getSelectedPosition('w').value() == Position{0, 0});
    REQUIRE(gameState.getSelectedPosition('b').value() == Position{1, 0});
}

TEST_CASE("a resting piece cannot jump") {
    Board board(3, 3);
    board.setCell(0, 0, "wR");
    GameState gameState(board);
    gameState.startJump(Position{0, 0});
    gameState.advanceTime(GameState::JUMP_DURATION_MS + 1);
    RealTimeArbiter::applyExpiredJumps(gameState);

    std::optional<Position> result = Controller::handleJump(0, 0, gameState, 'w');
    REQUIRE(result.has_value() == false);
}
