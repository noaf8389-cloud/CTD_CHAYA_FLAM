#pragma once
#include "../model/Position.hpp"
#include "../model/board.hpp"

class MovementHelper {
public:
    static bool isStraightLine(const Position& from, const Position& to);
    static bool isDiagonal(const Position& from, const Position& to);
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
    bool isLegalMove(const Position& from, const Position& to, const Board& board) const override;
};

class BishopRule : public PieceRule {
public:
    bool isLegalMove(const Position& from, const Position& to, const Board& board) const override;
};

class QueenRule : public PieceRule {
public:
    bool isLegalMove(const Position& from, const Position& to, const Board& board) const override;
};

class KingRule : public PieceRule {
public:
    bool isLegalMove(const Position& from, const Position& to, const Board& board) const override;
};

class KnightRule : public PieceRule {
public:
    bool isLegalMove(const Position& from, const Position& to, const Board& board) const override;
};

class PawnRule : public PieceRule {
public:
    bool isLegalMove(const Position& from, const Position& to, const Board& board) const override;
};

class RulesEngine {
public:
    static bool isLegalMove(const Position& from, const Position& to, const Board& board);
};
