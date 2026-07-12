#include "../catch2/catch_amalgamated.hpp"
#include "../../rules/rook_rule.hpp"

TEST_CASE("rook can move horizontally on a clear path") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    REQUIRE(RookRule::isLegalMove(Position{0, 0}, Position{0, 3}, board) == true);
}

TEST_CASE("rook can move vertically on a clear path") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    REQUIRE(RookRule::isLegalMove(Position{0, 0}, Position{3, 0}, board) == true);
}

TEST_CASE("rook cannot move diagonally") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    REQUIRE(RookRule::isLegalMove(Position{0, 0}, Position{3, 3}, board) == false);
}

TEST_CASE("rook cannot move in an L shape") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    REQUIRE(RookRule::isLegalMove(Position{0, 0}, Position{1, 2}, board) == false);
}

TEST_CASE("rook cannot jump over a piece in its path") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 2, "bK");
    REQUIRE(RookRule::isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("rook can capture an enemy piece directly at the destination") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 3, "bK");
    REQUIRE(RookRule::isLegalMove(Position{0, 0}, Position{0, 3}, board) == true);
}

TEST_CASE("rook can move from right to left on a clear path") {
    Board board(4, 4);
    board.setCell(0, 3, "wR");
    REQUIRE(RookRule::isLegalMove(Position{0, 3}, Position{0, 0}, board) == true);
}

TEST_CASE("rook cannot jump over a piece when moving vertically") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(2, 0, "bK");
    REQUIRE(RookRule::isLegalMove(Position{0, 0}, Position{3, 0}, board) == false);
}
