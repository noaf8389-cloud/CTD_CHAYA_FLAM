#include "../catch2/catch_amalgamated.hpp"
#include "../../rules/rules_engine.hpp"

TEST_CASE("rules engine dispatches rook moves to the rook rule") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{0, 3}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{3, 3}, board) == false);
}

TEST_CASE("rules engine dispatches bishop moves to the bishop rule") {
    Board board(4, 4);
    board.setCell(0, 0, "wB");
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{3, 3}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("rules engine dispatches queen moves to the queen rule") {
    Board board(4, 4);
    board.setCell(0, 0, "wQ");
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{0, 3}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{3, 3}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{2, 1}, board) == false);
}

TEST_CASE("rules engine dispatches king moves to the king rule") {
    Board board(4, 4);
    board.setCell(1, 1, "wK");
    REQUIRE(RulesEngine::isLegalMove(Position{1, 1}, Position{2, 2}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{1, 1}, Position{3, 3}, board) == false);
}

TEST_CASE("rules engine dispatches knight moves to the knight rule") {
    Board board(4, 4);
    board.setCell(0, 0, "wN");
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{1, 2}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("rules engine rejects a move from an empty cell") {
    Board board(4, 4);
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("rules engine rejects an unrecognized piece type such as a pawn") {
    Board board(4, 4);
    board.setCell(0, 0, "wP");
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{1, 0}, board) == false);
}
