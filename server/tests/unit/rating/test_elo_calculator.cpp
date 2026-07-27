#include "../../catch2/catch_amalgamated.hpp"
#include "../../../rating/elo_calculator.hpp"

TEST_CASE("equal ratings: winner gains half the k-factor, loser loses half") {
    EloResult result = calculateElo(1200, 1200, 32);
    REQUIRE(result.newWinnerRating == 1216);
    REQUIRE(result.newLoserRating == 1184);
}

TEST_CASE("a lower-rated winner gains more than half the k-factor") {
    EloResult result = calculateElo(1000, 1400, 32);
    REQUIRE(result.newWinnerRating > 1016);
}

TEST_CASE("a higher-rated winner gains less than half the k-factor") {
    EloResult result = calculateElo(1400, 1000, 32);
    REQUIRE(result.newWinnerRating < 1416);
}

TEST_CASE("a k-factor of zero produces no rating change") {
    EloResult result = calculateElo(1200, 1200, 0);
    REQUIRE(result.newWinnerRating == 1200);
    REQUIRE(result.newLoserRating == 1200);
}

TEST_CASE("a huge underdog upset yields close to the full k-factor gain") {
    EloResult result = calculateElo(800, 2000, 32);
    REQUIRE(result.newWinnerRating - 800 > 30);
}

TEST_CASE("a near-certain expected win yields close to zero rating gain") {
    EloResult result = calculateElo(2000, 800, 32);
    REQUIRE(result.newWinnerRating - 2000 < 2);
}
