#include "../catch2/catch_amalgamated.hpp"
#include "../../rules/bishop_rule.hpp"

TEST_CASE("bishop can move diagonally on a clear path") {
    Board board(4, 4);
    board.setCell(0, 0, "wB");
    REQUIRE(BishopRule::isLegalMove(Position{0, 0}, Position{3, 3}, board) == true);
}

TEST_CASE("bishop cannot move in a straight line") {
    Board board(4, 4);
    board.setCell(0, 0, "wB");
    REQUIRE(BishopRule::isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("bishop cannot jump over a piece on the diagonal") {
    Board board(4, 4);
    board.setCell(0, 0, "wB");
    board.setCell(1, 1, "wK");
    REQUIRE(BishopRule::isLegalMove(Position{0, 0}, Position{3, 3}, board) == false);
}

TEST_CASE("bishop can capture an enemy piece directly at the destination") {
    Board board(4, 4);
    board.setCell(0, 0, "wB");
    board.setCell(3, 3, "bK");
    REQUIRE(BishopRule::isLegalMove(Position{0, 0}, Position{3, 3}, board) == true);
}

TEST_CASE("bishop can move along the anti-diagonal") {
    Board board(4, 4);
    board.setCell(0, 3, "wB");
    REQUIRE(BishopRule::isLegalMove(Position{0, 3}, Position{3, 0}, board) == true);
}

TEST_CASE("bishop cannot jump over a piece on the anti-diagonal") {
    Board board(4, 4);
    board.setCell(0, 3, "wB");
    board.setCell(1, 2, "wK");
    REQUIRE(BishopRule::isLegalMove(Position{0, 3}, Position{3, 0}, board) == false);
}
