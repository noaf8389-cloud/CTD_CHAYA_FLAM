#pragma once
#include "../model/Position.hpp"
#include "../model/board.hpp"

class MovementHelper {
public:
    // Checks whether two positions lie on the same row or the same column.
    static bool isStraightLine(const Position& from, const Position& to);
    // Checks whether two positions lie on a common diagonal.
    static bool isDiagonal(const Position& from, const Position& to);
    // Checks that every cell strictly between two positions (along a straight or diagonal line) is empty.
    static bool isPathClear(const Position& from, const Position& to, const Board& board);

private:
    static int step(int from, int to);
};

class PieceRule {
public:
    virtual ~PieceRule() = default;
    virtual bool isLegalMove(const Position& from, const Position& to, const Board& board) const = 0;
};

class RookRule : public PieceRule {
public:
    // Legal if the move is a straight line with a clear path.
    bool isLegalMove(const Position& from, const Position& to, const Board& board) const override;
};

class BishopRule : public PieceRule {
public:
    // Legal if the move is diagonal with a clear path.
    bool isLegalMove(const Position& from, const Position& to, const Board& board) const override;
};

class QueenRule : public PieceRule {
public:
    // Legal if the move is a straight line or diagonal with a clear path.
    bool isLegalMove(const Position& from, const Position& to, const Board& board) const override;
};

class KingRule : public PieceRule {
public:
    // Legal if the destination is exactly one cell away in any direction.
    bool isLegalMove(const Position& from, const Position& to, const Board& board) const override;
};

class KnightRule : public PieceRule {
public:
    // Legal if the move is an L-shape: two cells one way and one cell perpendicular.
    bool isLegalMove(const Position& from, const Position& to, const Board& board) const override;
};

class PawnRule : public PieceRule {
public:
    // Legal for a single forward step, a two-step advance from the starting row, or a diagonal capture.
    bool isLegalMove(const Position& from, const Position& to, const Board& board) const override;
    // Checks whether the given row is the last row a pawn moving in this direction can reach.
    bool isPromotionRow(int forwardStep, int row, int rowCount) const;

private:
    bool isStartingRow(int forwardStep, int row, int rowCount) const;
    bool isForwardPathClear(const Position& from, int forwardStep, const Board& board) const;
};

class RulesEngine {
public:
    // Looks up the piece at `from` and checks whether moving it to `to` is legal under its own rule.
    static bool isLegalMove(const Position& from, const Position& to, const Board& board);
};
