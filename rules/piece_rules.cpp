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