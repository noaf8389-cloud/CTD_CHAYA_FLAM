#include "../catch2/catch_amalgamated.hpp"
#include "../../rules/knight_rule.hpp"

TEST_CASE("knight can move in all eight L shapes") {
    Board board(5, 5);
    Position center{2, 2};
    REQUIRE(KnightRule::isLegalMove(center, Position{0, 1}, board) == true);
    REQUIRE(KnightRule::isLegalMove(center, Position{0, 3}, board) == true);
    REQUIRE(KnightRule::isLegalMove(center, Position{1, 0}, board) == true);
    REQUIRE(KnightRule::isLegalMove(center, Position{1, 4}, board) == true);
    REQUIRE(KnightRule::isLegalMove(center, Position{3, 0}, board) == true);
    REQUIRE(KnightRule::isLegalMove(center, Position{3, 4}, board) == true);
    REQUIRE(KnightRule::isLegalMove(center, Position{4, 1}, board) == true);
    REQUIRE(KnightRule::isLegalMove(center, Position{4, 3}, board) == true);
}

TEST_CASE("knight cannot move in a straight line") {
    Board board(5, 5);
    REQUIRE(KnightRule::isLegalMove(Position{2, 2}, Position{2, 4}, board) == false);
}

TEST_CASE("knight cannot move diagonally") {
    Board board(5, 5);
    REQUIRE(KnightRule::isLegalMove(Position{2, 2}, Position{4, 4}, board) == false);
}

TEST_CASE("knight cannot stay on the same cell") {
    Board board(5, 5);
    REQUIRE(KnightRule::isLegalMove(Position{2, 2}, Position{2, 2}, board) == false);
}
