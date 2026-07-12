#include "../catch2/catch_amalgamated.hpp"
#include "../../rules/movement_helper.hpp"

TEST_CASE("isStraightLine is true for horizontal movement") {
    REQUIRE(MovementHelper::isStraightLine(Position{0, 0}, Position{0, 3}) == true);
}

TEST_CASE("isStraightLine is true for vertical movement") {
    REQUIRE(MovementHelper::isStraightLine(Position{0, 0}, Position{3, 0}) == true);
}

TEST_CASE("isStraightLine is false when nothing moved") {
    REQUIRE(MovementHelper::isStraightLine(Position{2, 2}, Position{2, 2}) == false);
}

TEST_CASE("isStraightLine is false for diagonal movement") {
    REQUIRE(MovementHelper::isStraightLine(Position{0, 0}, Position{2, 2}) == false);
}

TEST_CASE("isDiagonal is true for equal row and column difference") {
    REQUIRE(MovementHelper::isDiagonal(Position{0, 0}, Position{3, 3}) == true);
    REQUIRE(MovementHelper::isDiagonal(Position{3, 0}, Position{0, 3}) == true);
}

TEST_CASE("isDiagonal is false when nothing moved") {
    REQUIRE(MovementHelper::isDiagonal(Position{1, 1}, Position{1, 1}) == false);
}

TEST_CASE("isDiagonal is false for straight line movement") {
    REQUIRE(MovementHelper::isDiagonal(Position{0, 0}, Position{0, 3}) == false);
}

TEST_CASE("isPathClear is true when there is nothing between the cells") {
    Board board(4, 4);
    REQUIRE(MovementHelper::isPathClear(Position{0, 0}, Position{0, 3}, board) == true);
}

TEST_CASE("isPathClear is true for adjacent cells") {
    Board board(4, 4);
    REQUIRE(MovementHelper::isPathClear(Position{0, 0}, Position{0, 1}, board) == true);
}

TEST_CASE("isPathClear is false when a friendly piece blocks the way") {
    Board board(4, 4);
    board.setCell(0, 1, "wK");
    REQUIRE(MovementHelper::isPathClear(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("isPathClear is false when an enemy piece blocks the way") {
    Board board(4, 4);
    board.setCell(0, 1, "bK");
    REQUIRE(MovementHelper::isPathClear(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("isPathClear works correctly along a diagonal") {
    Board board(4, 4);
    board.setCell(1, 1, "wQ");
    REQUIRE(MovementHelper::isPathClear(Position{0, 0}, Position{3, 3}, board) == false);
}

TEST_CASE("isPathClear works when moving right to left") {
    Board board(4, 4);
    board.setCell(0, 1, "wK");
    REQUIRE(MovementHelper::isPathClear(Position{0, 3}, Position{0, 0}, board) == false);
}

TEST_CASE("isPathClear works when moving bottom to top") {
    Board board(4, 4);
    board.setCell(1, 0, "wK");
    REQUIRE(MovementHelper::isPathClear(Position{3, 0}, Position{0, 0}, board) == false);
}

TEST_CASE("isPathClear works along the anti-diagonal") {
    Board board(4, 4);
    board.setCell(1, 2, "wK");
    REQUIRE(MovementHelper::isPathClear(Position{0, 3}, Position{3, 0}, board) == false);
}

TEST_CASE("isDiagonal is true along the anti-diagonal") {
    REQUIRE(MovementHelper::isDiagonal(Position{0, 3}, Position{3, 0}) == true);
}
