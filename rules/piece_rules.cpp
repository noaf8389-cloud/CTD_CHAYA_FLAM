#include "piece_rules.hpp"
#include "../model/board.hpp"

const std::string PieceRules::VALID_COLORS = "wb";
const std::string PieceRules::VALID_PIECE_TYPES = "KQRBNP";

bool PieceRules::isValidToken(const std::string& token) {
    if (token == Board::EMPTY_CELL) {
        return true;
    }
    if (token.size() != 2) {
        return false;
    }
    bool validColor = VALID_COLORS.find(token[0]) != std::string::npos;
    bool validPiece = VALID_PIECE_TYPES.find(token[1]) != std::string::npos;
    return validColor && validPiece;
}

bool PieceRules::isSameColor(const std::string& token1, const std::string& token2) {
    if (token1 == Board::EMPTY_CELL || token2 == Board::EMPTY_CELL) {
        return false;
    }
    return token1[0] == token2[0];
}

bool PieceRules::isColor(const std::string& token, char color) {
    if (token == Board::EMPTY_CELL) {
        return false;
    }
    return token[0] == color;
}
