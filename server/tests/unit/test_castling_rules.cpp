#include "../catch2/catch_amalgamated.hpp"
#include "../../logic/rules/castling_rules.hpp"

namespace {
    Board makeCastlingBoard() {
        Board board(8, 8);
        board.setCell(7, 4, "wK");
        board.setCell(7, 0, "wR");
        board.setCell(7, 7, "wR");
        return board;
    }

    bool noneVacated(const Position&) { return false; }
}

TEST_CASE("kingside castling is legal when nothing has moved and the path is clear") {
    Board board = makeCastlingBoard();
    auto result = CastlingRules::tryCastle(Position{7, 4}, Position{7, 6}, board, noneVacated);

    REQUIRE(result.has_value());
    REQUIRE(result->kingFrom == Position{7, 4});
    REQUIRE(result->kingTo == Position{7, 6});
    REQUIRE(result->rookFrom == Position{7, 7});
    REQUIRE(result->rookTo == Position{7, 5});
}

TEST_CASE("queenside castling is legal when nothing has moved and the path is clear") {
    Board board = makeCastlingBoard();
    auto result = CastlingRules::tryCastle(Position{7, 4}, Position{7, 2}, board, noneVacated);

    REQUIRE(result.has_value());
    REQUIRE(result->rookFrom == Position{7, 0});
    REQUIRE(result->rookTo == Position{7, 3});
}

TEST_CASE("castling is illegal if the king has ever moved") {
    Board board = makeCastlingBoard();
    auto wasVacated = [](const Position& p) { return p == Position{7, 4}; };
    auto result = CastlingRules::tryCastle(Position{7, 4}, Position{7, 6}, board, wasVacated);

    REQUIRE(result.has_value() == false);
}

TEST_CASE("castling is illegal if the specific rook has ever moved") {
    Board board = makeCastlingBoard();
    auto wasVacated = [](const Position& p) { return p == Position{7, 7}; };
    auto result = CastlingRules::tryCastle(Position{7, 4}, Position{7, 6}, board, wasVacated);

    REQUIRE(result.has_value() == false);
}

TEST_CASE("castling is legal on one side even if the other side's rook has moved") {
    Board board = makeCastlingBoard();
    auto wasVacated = [](const Position& p) { return p == Position{7, 0}; };
    auto result = CastlingRules::tryCastle(Position{7, 4}, Position{7, 6}, board, wasVacated);

    REQUIRE(result.has_value());
}

TEST_CASE("castling is illegal if a piece stands between king and rook") {
    Board board = makeCastlingBoard();
    board.setCell(7, 5, "wB");
    auto result = CastlingRules::tryCastle(Position{7, 4}, Position{7, 6}, board, noneVacated);

    REQUIRE(result.has_value() == false);
}

TEST_CASE("castling is illegal if the selected piece is not a king") {
    Board board = makeCastlingBoard();
    auto result = CastlingRules::tryCastle(Position{7, 0}, Position{7, 2}, board, noneVacated);

    REQUIRE(result.has_value() == false);
}

TEST_CASE("castling is illegal if there is no rook on the corner square") {
    Board board(8, 8);
    board.setCell(7, 4, "wK");
    auto result = CastlingRules::tryCastle(Position{7, 4}, Position{7, 6}, board, noneVacated);

    REQUIRE(result.has_value() == false);
}

TEST_CASE("castling is illegal if the corner piece is the wrong color") {
    Board board = makeCastlingBoard();
    board.setCell(7, 7, "bR");
    auto result = CastlingRules::tryCastle(Position{7, 4}, Position{7, 6}, board, noneVacated);

    REQUIRE(result.has_value() == false);
}

TEST_CASE("a one-square king move is not treated as castling") {
    Board board = makeCastlingBoard();
    auto result = CastlingRules::tryCastle(Position{7, 4}, Position{7, 5}, board, noneVacated);

    REQUIRE(result.has_value() == false);
}
