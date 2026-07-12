#include "rules_engine.hpp"
#include "rook_rule.hpp"
#include "bishop_rule.hpp"
#include "queen_rule.hpp"
#include "king_rule.hpp"
#include "knight_rule.hpp"

bool RulesEngine::isLegalMove(const Position& from, const Position& to, const Board& board) {
    std::string token = board.getCell(from.row, from.col);
    if (token == Board::EMPTY_CELL) {
        return false;
    }

    char pieceType = token[1];

    switch (pieceType) {
        case 'R': return RookRule::isLegalMove(from, to, board);
        case 'B': return BishopRule::isLegalMove(from, to, board);
        case 'Q': return QueenRule::isLegalMove(from, to, board);
        case 'K': return KingRule::isLegalMove(from, to, board);
        case 'N': return KnightRule::isLegalMove(from, to, board);
        default: return false;
    }
}
