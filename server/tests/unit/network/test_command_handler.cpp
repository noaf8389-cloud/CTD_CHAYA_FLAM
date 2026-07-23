#include "../../catch2/catch_amalgamated.hpp"
#include "../../../network/command_handler.hpp"
#include "../../../game_session.hpp"

TEST_CASE("CommandHandler click command selects a piece") {
    Board board(3, 3);
    board.setCell(0, 0, "wR");
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);
    CommandHandler handler(session);

    handler.handleMessage(R"({"command":"click","row":0,"col":0})");

    REQUIRE(gameState.getSelectedPosition().has_value());
    REQUIRE(gameState.getSelectedPosition()->row == 0);
    REQUIRE(gameState.getSelectedPosition()->col == 0);
}

TEST_CASE("CommandHandler ignores malformed JSON without crashing") {
    Board board(3, 3);
    board.setCell(0, 0, "wR");
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);
    CommandHandler handler(session);
    handler.handleMessage("not valid json");

    REQUIRE(gameState.getSelectedPosition().has_value() == false);
}

TEST_CASE("CommandHandler ignores unknown command") {
    Board board(3, 3);
    board.setCell(0, 0, "wR");
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);
    CommandHandler handler(session);
    handler.handleMessage(R"({"command":"foobar","row":0,"col":0})");

    REQUIRE(gameState.getSelectedPosition().has_value() == false);
}
