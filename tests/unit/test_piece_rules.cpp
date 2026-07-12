#include "../catch2/catch_amalgamated.hpp"
#include "../../rules/piece_rules.hpp"

TEST_CASE("empty cell token is valid") {
    REQUIRE(PieceRules::isValidToken(".") == true);
}

TEST_CASE("valid piece tokens are valid") {
    REQUIRE(PieceRules::isValidToken("wK") == true);
    REQUIRE(PieceRules::isValidToken("bQ") == true);
}

TEST_CASE("unknown token is invalid") {
    REQUIRE(PieceRules::isValidToken("xZ") == false);
}

TEST_CASE("wrong length token is invalid") {
    REQUIRE(PieceRules::isValidToken("wKK") == false);
    REQUIRE(PieceRules::isValidToken("w") == false);
}

TEST_CASE("empty string token is invalid") {
    REQUIRE(PieceRules::isValidToken("") == false);
}

TEST_CASE("valid color with invalid piece type is invalid") {
    REQUIRE(PieceRules::isValidToken("wX") == false);
}

TEST_CASE("invalid color with valid piece type is invalid") {
    REQUIRE(PieceRules::isValidToken("xK") == false);
}

TEST_CASE("lowercase piece letter is invalid") {
    REQUIRE(PieceRules::isValidToken("wk") == false);
}

TEST_CASE("uppercase color letter is invalid") {
    REQUIRE(PieceRules::isValidToken("WK") == false);
}

TEST_CASE("same color pieces are recognized as same color") {
    REQUIRE(PieceRules::isSameColor("wK", "wQ") == true);
    REQUIRE(PieceRules::isSameColor("bK", "bR") == true);
}

TEST_CASE("different color pieces are not same color") {
    REQUIRE(PieceRules::isSameColor("wK", "bQ") == false);
}

TEST_CASE("empty cell is never the same color as anything") {
    REQUIRE(PieceRules::isSameColor(".", "wK") == false);
    REQUIRE(PieceRules::isSameColor("wK", ".") == false);
    REQUIRE(PieceRules::isSameColor(".", ".") == false);
}

TEST_CASE("isColor matches the token color correctly") {
    REQUIRE(PieceRules::isColor("wK", 'w') == true);
    REQUIRE(PieceRules::isColor("wK", 'b') == false);
    REQUIRE(PieceRules::isColor(".", 'w') == false);
}
