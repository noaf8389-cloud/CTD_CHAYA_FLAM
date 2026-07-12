#include "../catch2/catch_amalgamated.hpp"
#include "../../rules/king_rule.hpp"

TEST_CASE("king can move one cell in any of the eight directions") {
    Board board(3, 3);
    REQUIRE(KingRule::isLegalMove(Position{1, 1}, Position{0, 0}, board) == true);
    REQUIRE(KingRule::isLegalMove(Position{1, 1}, Position{0, 1}, board) == true);
    REQUIRE(KingRule::isLegalMove(Position{1, 1}, Position{0, 2}, board) == true);
    REQUIRE(KingRule::isLegalMove(Position{1, 1}, Position{1, 0}, board) == true);
    REQUIRE(KingRule::isLegalMove(Position{1, 1}, Position{1, 2}, board) == true);
    REQUIRE(KingRule::isLegalMove(Position{1, 1}, Position{2, 0}, board) == true);
    REQUIRE(KingRule::isLegalMove(Position{1, 1}, Position{2, 1}, board) == true);
    REQUIRE(KingRule::isLegalMove(Position{1, 1}, Position{2, 2}, board) == true);
}

TEST_CASE("king cannot move two cells in a straight line") {
    Board board(4, 4);
    REQUIRE(KingRule::isLegalMove(Position{0, 0}, Position{0, 2}, board) == false);
}

TEST_CASE("king cannot move two cells diagonally") {
    Board board(4, 4);
    REQUIRE(KingRule::isLegalMove(Position{0, 0}, Position{2, 2}, board) == false);
}

TEST_CASE("king cannot stay on the same cell") {
    Board board(3, 3);
    REQUIRE(KingRule::isLegalMove(Position{1, 1}, Position{1, 1}, board) == false);
}
