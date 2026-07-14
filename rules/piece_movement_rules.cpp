#include "piece_movement_rules.hpp"
#include "piece_rules.hpp"
#include <cstdlib>
#include <map>
#include <memory>

int MovementHelper::step(int from, int to) {
    if (to > from) return 1;
    if (to < from) return -1;
    return 0;
}

bool MovementHelper::isStraightLine(const Position& from, const Position& to) {
    bool sameRow = from.row == to.row;
    bool sameCol = from.col == to.col;
    return sameRow != sameCol;
}

bool MovementHelper::isDiagonal(const Position& from, const Position& to) {
    int rowDiff = std::abs(from.row - to.row);
    int colDiff = std::abs(from.col - to.col);
    return rowDiff == colDiff && rowDiff != 0;
}

bool MovementHelper::isPathClear(const Position& from, const Position& to, const Board& board) {
    int rowStep = step(from.row, to.row);
    int colStep = step(from.col, to.col);

    int row = from.row + rowStep;
    int col = from.col + colStep;

    while (row != to.row || col != to.col) {
        if (board.getCell(row, col) != Board::EMPTY_CELL) {
            return false;
        }
        row += rowStep;
        col += colStep;
    }

    return true;
}

bool RookRule::isLegalMove(const Position& from, const Position& to, const Board& board) const {
    if (!MovementHelper::isStraightLine(from, to)) {
        return false;
    }
    return MovementHelper::isPathClear(from, to, board);
}

bool BishopRule::isLegalMove(const Position& from, const Position& to, const Board& board) const {
    if (!MovementHelper::isDiagonal(from, to)) {
        return false;
    }
    return MovementHelper::isPathClear(from, to, board);
}

bool QueenRule::isLegalMove(const Position& from, const Position& to, const Board& board) const {
    bool validShape = MovementHelper::isStraightLine(from, to) || MovementHelper::isDiagonal(from, to);
    if (!validShape) {
        return false;
    }
    return MovementHelper::isPathClear(from, to, board);
}

bool KingRule::isLegalMove(const Position& from, const Position& to, const Board& board) const {
    int rowDiff = std::abs(from.row - to.row);
    int colDiff = std::abs(from.col - to.col);

    if (rowDiff == 0 && colDiff == 0) {
        return false;
    }

    return rowDiff <= 1 && colDiff <= 1;
}

bool KnightRule::isLegalMove(const Position& from, const Position& to, const Board& board) const {
    int rowDiff = std::abs(from.row - to.row);
    int colDiff = std::abs(from.col - to.col);

    return (rowDiff == 1 && colDiff == 2) || (rowDiff == 2 && colDiff == 1);
}

// PawnRule

bool PawnRule::isStartingRow(int forwardStep, int row, int rowCount) const {
    if (forwardStep == -1) {
        return row == rowCount - 2;
    }
    return row == 1;
}

bool PawnRule::isForwardPathClear(const Position& from, int forwardStep, const Board& board) const {
    Position middle{from.row + forwardStep, from.col};
    Position destination{from.row + 2 * forwardStep, from.col};
    return board.getCell(middle.row, middle.col) == Board::EMPTY_CELL
        && board.getCell(destination.row, destination.col) == Board::EMPTY_CELL;
}

bool PawnRule::isPromotionRow(int forwardStep, int row, int rowCount) const {
    if (forwardStep == -1) {
        return row == 0;
    }
    return row == rowCount - 1;
}

bool PawnRule::isLegalMove(const Position& from, const Position& to, const Board& board) const {
    std::string movingToken = board.getCell(from.row, from.col);
    char color = movingToken[0];
    int forwardStep = PieceRules::getForwardDirection(color);

    if (isPromotionRow(forwardStep, from.row, board.getRowCount())) {
        return false;
    }

    int rowDiff = to.row - from.row;
    int colDiff = to.col - from.col;

    // תנועה אלכסונית - זז שורה אחת ועמודה אחת
    if (colDiff != 0) {
        if (rowDiff != forwardStep) {
            return false;
        }
        if (colDiff != 1 && colDiff != -1) {
            return false;
        }
        return board.getCell(to.row, to.col) != Board::EMPTY_CELL;
    }

    if (rowDiff == forwardStep) {
        return board.getCell(to.row, to.col) == Board::EMPTY_CELL;
    }

    if (rowDiff == 2 * forwardStep && isStartingRow(forwardStep, from.row, board.getRowCount())) {
        return isForwardPathClear(from, forwardStep, board);
    }

    return false;
}

namespace {
    const std::map<char, std::unique_ptr<PieceRule>>& getRuleRegistry() {
        static const std::map<char, std::unique_ptr<PieceRule>> registry = [] {
            std::map<char, std::unique_ptr<PieceRule>> map;
            map['R'] = std::make_unique<RookRule>();
            map['B'] = std::make_unique<BishopRule>();
            map['Q'] = std::make_unique<QueenRule>();
            map['K'] = std::make_unique<KingRule>();
            map['N'] = std::make_unique<KnightRule>();
            map['P'] = std::make_unique<PawnRule>();
            return map;
        }();
        return registry;
    }
}

bool RulesEngine::isLegalMove(const Position& from, const Position& to, const Board& board) {
    std::string token = board.getCell(from.row, from.col);
    if (token == Board::EMPTY_CELL) {
        return false;
    }

    std::string destinationToken = board.getCell(to.row, to.col);
    if (PieceRules::isSameColor(token, destinationToken)) {
        return false;
    }

    char pieceType = token[1];

    const auto& registry = getRuleRegistry();
    auto it = registry.find(pieceType);
    if (it == registry.end()) {
        return false;
    }

    return it->second->isLegalMove(from, to, board);
}
