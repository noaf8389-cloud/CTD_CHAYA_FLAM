#include "../catch2/catch_amalgamated.hpp"
#include "../../logic/input/board_mapper.hpp"

TEST_CASE("valid row/col within bounds returns the matching position") {
    auto position = BoardMapper::toPosition(1, 2, 3, 3);
    REQUIRE(position.has_value());
    REQUIRE(position->row == 1);
    REQUIRE(position->col == 2);
}

TEST_CASE("negative row is out of bounds") {
    REQUIRE(BoardMapper::isOutOfBounds(-1, 0, 3, 3) == true);
    REQUIRE(BoardMapper::toPosition(-1, 0, 3, 3) == std::nullopt);
}

TEST_CASE("negative col is out of bounds") {
    REQUIRE(BoardMapper::isOutOfBounds(0, -1, 3, 3) == true);
}

TEST_CASE("row beyond board height is out of bounds") {
    REQUIRE(BoardMapper::isOutOfBounds(3, 0, 3, 3) == true);
    REQUIRE(BoardMapper::toPosition(3, 0, 3, 3) == std::nullopt);
}

TEST_CASE("col beyond board width is out of bounds") {
    REQUIRE(BoardMapper::isOutOfBounds(0, 3, 3, 3) == true);
}

TEST_CASE("zero sized board rejects every position") {
    REQUIRE(BoardMapper::isOutOfBounds(0, 0, 0, 0) == true);
}
