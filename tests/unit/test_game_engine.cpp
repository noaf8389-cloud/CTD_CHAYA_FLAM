#include <iostream>
#include "../catch2/catch_amalgamated.hpp"
#include <sstream>
#include "../../io/board_parser.hpp"
#include "../../io/board_print.hpp"
#include "../../model/game_state.hpp"
#include "../../engine/game_engine.hpp"

namespace {
    std::string runScenario(const std::string& input) {
        std::istringstream in(input);
        Board board(0, 0);
        std::vector<std::string> commands;
        BoardParser::parse(in, board, commands);

        GameState gameState(board);

        std::ostringstream captured;
        std::streambuf* originalBuffer = std::cout.rdbuf(captured.rdbuf());
        GameEngine::run(gameState, commands);
        std::cout.rdbuf(originalBuffer);

        return captured.str();
    }
}

TEST_CASE("click selects and moves a piece across two cells") {
    std::string output = runScenario(
        "Board:\nwK . .\n. . .\n. . .\nCommands:\nclick 50 50\nclick 150 150\nwait 1000\nprint board\n");
    REQUIRE(output == ". . .\n. wK .\n. . .\n");
}

TEST_CASE("click_empty_cell_does_not_select") {
    std::string output = runScenario(
        "Board:\nwK . .\n. . .\n. . .\nCommands:\nclick 150 150\nclick 250 250\nwait 1000\nprint board\n");
    REQUIRE(output == "wK . .\n. . .\n. . .\n");
}

TEST_CASE("click_outside_board_is_ignored") {
    std::string output = runScenario(
        "Board:\nwK . .\n. . .\n. . .\nCommands:\nclick 350 50\nclick -10 50\nprint board\n");
    REQUIRE(output == "wK . .\n. . .\n. . .\n");
}

TEST_CASE("clicking_another_piece_replaces_selection") {
    std::string output = runScenario(
        "Board:\nwR . wK\n. . .\nCommands:\nclick 50 50\nclick 250 50\nclick 250 150\nwait 1000\nprint board\n");
    REQUIRE(output == "wR . .\n. . wK\n");
}

TEST_CASE("two pieces can have pending moves at the same time") {
    std::string output = runScenario(
        "Board:\nwK . wQ\n. . .\n. . .\nCommands:\nclick 50 50\nclick 50 150\nclick 250 50\nclick 250 150\nwait 1000\nprint board\n");
    REQUIRE(output == ". . .\nwK . wQ\n. . .\n");
}

TEST_CASE("malformed wait command without a number is ignored") {
    std::string output = runScenario("Board:\nwK\nCommands:\nwait abc\nprint board\n");
    REQUIRE(output == "wK\n");
}

TEST_CASE("malformed click command missing y is ignored") {
    std::string output = runScenario("Board:\nwK\nCommands:\nclick 50\nprint board\n");
    REQUIRE(output == "wK\n");
}

TEST_CASE("completely unknown command is ignored without crashing") {
    std::string output = runScenario("Board:\nwK\nCommands:\nfoobar\nprint board\n");
    REQUIRE(output == "wK\n");
}

TEST_CASE("print board can be called multiple times") {
    std::string output = runScenario("Board:\nwK\nCommands:\nprint board\nprint board\n");
    REQUIRE(output == "wK\nwK\n");
}

TEST_CASE("a legal move through the full engine updates the board") {
    std::string output = runScenario(
        "Board:\nwR . . .\n. . . .\n. . . .\n. . . .\nCommands:\nclick 50 50\nclick 350 50\nwait 3000\nprint board\n");
    REQUIRE(output == ". . . wR\n. . . .\n. . . .\n. . . .\n");
}

TEST_CASE("one cell move is not complete before its full duration has passed") {
    std::string output = runScenario(
        "Board:\nwR . .\nCommands:\nclick 50 50\nclick 150 50\nwait 500\nprint board\n");
    REQUIRE(output == "wR . .\n");
}

TEST_CASE("two cell move shows original position before arrival and destination after") {
    std::string output = runScenario(
        "Board:\nwR . .\nCommands:\nclick 50 50\nclick 250 50\nwait 1000\nprint board\nwait 1000\nprint board\n");
    REQUIRE(output == "wR . .\n. . wR\n");
}
