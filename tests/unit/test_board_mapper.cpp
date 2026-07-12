#include "../catch2/catch_amalgamated.hpp"
#include "../../input/board_mapper.hpp"

TEST_CASE("click at top-left corner maps to cell (0,0)") {
    auto position = BoardMapper::toPosition(50, 50, 3, 3);
    REQUIRE(position.has_value());
    REQUIRE(position->row == 0);
    REQUIRE(position->col == 0);
}

TEST_CASE("click maps to correct cell by column and row") {
    auto position = BoardMapper::toPosition(150, 250, 3, 3);
    REQUIRE(position.has_value());
    REQUIRE(position->row == 2);
    REQUIRE(position->col == 1);
}

TEST_CASE("click exactly on cell boundary belongs to the next cell") {
    auto position = BoardMapper::toPosition(100, 0, 3, 3);
    REQUIRE(position.has_value());
    REQUIRE(position->col == 1);
    REQUIRE(position->row == 0);
}

TEST_CASE("click just before boundary still belongs to the previous cell") {
    auto position = BoardMapper::toPosition(99, 0, 3, 3);
    REQUIRE(position.has_value());
    REQUIRE(position->col == 0);
}

TEST_CASE("negative x is out of bounds") {
    REQUIRE(BoardMapper::isOutOfBounds(-10, 50, 3, 3) == true);
    REQUIRE(BoardMapper::toPosition(-10, 50, 3, 3) == std::nullopt);
}

TEST_CASE("negative y is out of bounds") {
    REQUIRE(BoardMapper::isOutOfBounds(50, -1, 3, 3) == true);
}

TEST_CASE("x beyond board width is out of bounds") {
    REQUIRE(BoardMapper::isOutOfBounds(350, 50, 3, 3) == true);
    REQUIRE(BoardMapper::toPosition(350, 50, 3, 3) == std::nullopt);
}

TEST_CASE("y beyond board height is out of bounds") {
    REQUIRE(BoardMapper::isOutOfBounds(50, 350, 3, 3) == true);
}

TEST_CASE("click exactly on the outer edge of the board is out of bounds") {
    REQUIRE(BoardMapper::isOutOfBounds(300, 0, 3, 3) == true);
}

TEST_CASE("zero sized board rejects every click") {
    REQUIRE(BoardMapper::isOutOfBounds(0, 0, 0, 0) == true);
}

TEST_CASE("y beyond board height returns no position") {
    REQUIRE(BoardMapper::toPosition(50, 350, 3, 3) == std::nullopt);
}
