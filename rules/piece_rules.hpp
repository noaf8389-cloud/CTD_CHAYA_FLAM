#pragma once
#include <string>

class PieceRules {
public:
    static bool isValidToken(const std::string& token);
    static bool isSameColor(const std::string& token1, const std::string& token2);
    static bool isColor(const std::string& token, char color);
    static int getForwardDirection(char color) {
    return (color == 'w') ? -1 : 1;
}

private:
    static const std::string VALID_COLORS;
    static const std::string VALID_PIECE_TYPES;
};