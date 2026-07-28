#include "castling_rules.hpp"
#include "piece_movement_rules.hpp"

#include <cstdlib>

std::optional<CastlingMove> CastlingRules::tryCastle(const Position& from, const Position& to, const Board& board,
                                                      const std::function<bool(const Position&)>& wasVacated) {
    std::string kingToken = board.getCell(from.row, from.col);
    if (kingToken.size() != 2 || kingToken[1] != 'K') {
        return std::nullopt;
    }

    if (from.row != to.row || std::abs(from.col - to.col) != 2) {
        return std::nullopt;
    }

    if (wasVacated(from)) {
        return std::nullopt;
    }

    bool kingSide = to.col > from.col;
    int rookCol = kingSide ? board.getColCount() - 1 : 0;
    Position rookFrom{from.row, rookCol};

    std::string rookToken = board.getCell(rookFrom.row, rookFrom.col);
    if (rookToken.size() != 2 || rookToken[1] != 'R' || rookToken[0] != kingToken[0]) {
        return std::nullopt;
    }

    if (wasVacated(rookFrom)) {
        return std::nullopt;
    }

    if (!MovementHelper::isPathClear(from, rookFrom, board)) {
        return std::nullopt;
    }

    Position rookTo{from.row, kingSide ? to.col - 1 : to.col + 1};
    return CastlingMove{from, to, rookFrom, rookTo};
}
