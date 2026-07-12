#include "../catch2/catch_amalgamated.hpp"
#include "../../rules/queen_rule.hpp"

TEST_CASE("queen can move in a straight line") {
    Board board(4, 4);
    board.setCell(0, 0, "wQ");
    REQUIRE(QueenRule::isLegalMove(Position{0, 0}, Position{0, 3}, board) == true);
}

TEST_CASE("queen can move diagonally") {
    Board board(4, 4);
    board.setCell(0, 0, "wQ");
    REQUIRE(QueenRule::isLegalMove(Position{0, 0}, Position{3, 3}, board) == true);
}

TEST_CASE("queen cannot move in an L shape") {
    Board board(4, 4);
    board.setCell(0, 0, "wQ");
    REQUIRE(QueenRule::isLegalMove(Position{0, 0}, Position{2, 1}, board) == false);
}

TEST_CASE("queen cannot jump over a piece in a straight line") {
    Board board(4, 4);
    board.setCell(0, 0, "wQ");
    board.setCell(0, 1, "wK");
    REQUIRE(QueenRule::isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("queen cannot jump over a piece on the diagonal") {
    Board board(4, 4);
    board.setCell(0, 0, "wQ");
    board.setCell(1, 1, "wK");
    REQUIRE(QueenRule::isLegalMove(Position{0, 0}, Position{3, 3}, board) == false);
}

TEST_CASE("queen can move along the anti-diagonal") {
    Board board(4, 4);
    board.setCell(0, 3, "wQ");
    REQUIRE(QueenRule::isLegalMove(Position{0, 3}, Position{3, 0}, board) == true);
}

TEST_CASE("queen can move from right to left") {
    Board board(4, 4);
    board.setCell(0, 3, "wQ");
    REQUIRE(QueenRule::isLegalMove(Position{0, 3}, Position{0, 0}, board) == true);
}
