#pragma once
#include <string>

class PieceRules {
public:
    // Checks that a token has a recognized color and piece-type character.
    static bool isValidToken(const std::string& token);
    // Checks whether two tokens belong to the same color.
    static bool isSameColor(const std::string& token1, const std::string& token2);
    // Checks whether a token belongs to the given color.
    static bool isColor(const std::string& token, char color);
    static int getForwardDirection(char color) {
    return (color == 'w') ? -1 : 1;
}

private:
    static const std::string VALID_COLORS;
    static const std::string VALID_PIECE_TYPES;
};