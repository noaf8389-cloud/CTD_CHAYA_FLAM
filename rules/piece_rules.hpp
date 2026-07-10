#pragma once
#include <string>

class PieceRules {
public:
    static bool isValidToken(const std::string& token);

private:
    static const std::string VALID_COLORS;
    static const std::string VALID_PIECE_TYPES;
};