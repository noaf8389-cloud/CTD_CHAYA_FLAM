#pragma once

struct EloResult {
    int newWinnerRating;
    int newLoserRating;
};

// Computes each player's updated ELO rating after one game, given the pre-game ratings of the winner and loser.
EloResult calculateElo(int winnerRating, int loserRating, int kFactor = 32);
