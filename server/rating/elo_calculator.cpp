#include "elo_calculator.hpp"
#include <cmath>

EloResult calculateElo(int winnerRating, int loserRating, int kFactor) {
    double expectedWinner = 1.0 / (1.0 + std::pow(10.0, (loserRating - winnerRating) / 400.0));
    double expectedLoser = 1.0 - expectedWinner;

    int newWinnerRating = winnerRating + static_cast<int>(std::round(kFactor * (1.0 - expectedWinner)));
    int newLoserRating = loserRating + static_cast<int>(std::round(kFactor * (0.0 - expectedLoser)));

    return EloResult{newWinnerRating, newLoserRating};
}
