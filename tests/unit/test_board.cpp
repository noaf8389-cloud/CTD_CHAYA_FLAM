#include "../catch2/catch_amalgamated.hpp"
#include "../../model/board.hpp"
#include <stdexcept>

TEST_CASE("new board cells default to empty") {
    Board board(2, 3);
    REQUIRE(board.getRowCount() == 2);
    REQUIRE(board.getColCount() == 3);
    REQUIRE(board.getCell(0, 0) == ".");
}

TEST_CASE("setCell stores the value at the right position") {
    Board board(2, 2);
    board.setCell(1, 0, "wK");
    REQUIRE(board.getCell(1, 0) == "wK");
    REQUIRE(board.getCell(0, 0) == ".");
}

TEST_CASE("out of range row throws") {
    Board board(2, 2);
    REQUIRE_THROWS_AS(board.getCell(5, 0), std::out_of_range);
    REQUIRE_THROWS_AS(board.setCell(-1, 0, "wK"), std::out_of_range);
}

TEST_CASE("out of range column throws") {
    Board board(2, 2);
    REQUIRE_THROWS_AS(board.getCell(0, 5), std::out_of_range);
}

TEST_CASE("zero sized board does not crash on construction") {
    Board board(0, 0);
    REQUIRE(board.getRowCount() == 0);
    REQUIRE(board.getColCount() == 0);
}