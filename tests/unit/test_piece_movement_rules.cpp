#include "../catch2/catch_amalgamated.hpp"
#include "../../rules/piece_movement_rules.hpp"

// ===== MovementHelper =====

TEST_CASE("isStraightLine is true for horizontal movement") {
    REQUIRE(MovementHelper::isStraightLine(Position{0, 0}, Position{0, 3}) == true);
}

TEST_CASE("isStraightLine is true for vertical movement") {
    REQUIRE(MovementHelper::isStraightLine(Position{0, 0}, Position{3, 0}) == true);
}

TEST_CASE("isStraightLine is false when nothing moved") {
    REQUIRE(MovementHelper::isStraightLine(Position{2, 2}, Position{2, 2}) == false);
}

TEST_CASE("isStraightLine is false for diagonal movement") {
    REQUIRE(MovementHelper::isStraightLine(Position{0, 0}, Position{2, 2}) == false);
}

TEST_CASE("isDiagonal is true for equal row and column difference") {
    REQUIRE(MovementHelper::isDiagonal(Position{0, 0}, Position{3, 3}) == true);
    REQUIRE(MovementHelper::isDiagonal(Position{3, 0}, Position{0, 3}) == true);
}

TEST_CASE("isDiagonal is false when nothing moved") {
    REQUIRE(MovementHelper::isDiagonal(Position{1, 1}, Position{1, 1}) == false);
}

TEST_CASE("isDiagonal is false for straight line movement") {
    REQUIRE(MovementHelper::isDiagonal(Position{0, 0}, Position{0, 3}) == false);
}

TEST_CASE("isDiagonal is true along the anti-diagonal") {
    REQUIRE(MovementHelper::isDiagonal(Position{0, 3}, Position{3, 0}) == true);
}

TEST_CASE("isPathClear is true when there is nothing between the cells") {
    Board board(4, 4);
    REQUIRE(MovementHelper::isPathClear(Position{0, 0}, Position{0, 3}, board) == true);
}

TEST_CASE("isPathClear is true for adjacent cells") {
    Board board(4, 4);
    REQUIRE(MovementHelper::isPathClear(Position{0, 0}, Position{0, 1}, board) == true);
}

TEST_CASE("isPathClear is false when a friendly piece blocks the way") {
    Board board(4, 4);
    board.setCell(0, 1, "wK");
    REQUIRE(MovementHelper::isPathClear(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("isPathClear is false when an enemy piece blocks the way") {
    Board board(4, 4);
    board.setCell(0, 1, "bK");
    REQUIRE(MovementHelper::isPathClear(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("isPathClear works correctly along a diagonal") {
    Board board(4, 4);
    board.setCell(1, 1, "wQ");
    REQUIRE(MovementHelper::isPathClear(Position{0, 0}, Position{3, 3}, board) == false);
}

TEST_CASE("isPathClear works when moving right to left") {
    Board board(4, 4);
    board.setCell(0, 1, "wK");
    REQUIRE(MovementHelper::isPathClear(Position{0, 3}, Position{0, 0}, board) == false);
}

TEST_CASE("isPathClear works when moving bottom to top") {
    Board board(4, 4);
    board.setCell(1, 0, "wK");
    REQUIRE(MovementHelper::isPathClear(Position{3, 0}, Position{0, 0}, board) == false);
}

TEST_CASE("isPathClear works along the anti-diagonal") {
    Board board(4, 4);
    board.setCell(1, 2, "wK");
    REQUIRE(MovementHelper::isPathClear(Position{0, 3}, Position{3, 0}, board) == false);
}

// ===== RookRule =====

TEST_CASE("rook can move horizontally on a clear path") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    REQUIRE(RookRule().isLegalMove(Position{0, 0}, Position{0, 3}, board) == true);
}

TEST_CASE("rook can move vertically on a clear path") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    REQUIRE(RookRule().isLegalMove(Position{0, 0}, Position{3, 0}, board) == true);
}

TEST_CASE("rook cannot move diagonally") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    REQUIRE(RookRule().isLegalMove(Position{0, 0}, Position{3, 3}, board) == false);
}

TEST_CASE("rook cannot move in an L shape") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    REQUIRE(RookRule().isLegalMove(Position{0, 0}, Position{1, 2}, board) == false);
}

TEST_CASE("rook cannot jump over a piece in its horizontal path") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 2, "bK");
    REQUIRE(RookRule().isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("rook cannot jump over a piece when moving vertically") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(2, 0, "bK");
    REQUIRE(RookRule().isLegalMove(Position{0, 0}, Position{3, 0}, board) == false);
}

TEST_CASE("rook can capture an enemy piece directly at the destination") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 3, "bK");
    REQUIRE(RookRule().isLegalMove(Position{0, 0}, Position{0, 3}, board) == true);
}

TEST_CASE("rook can move from right to left on a clear path") {
    Board board(4, 4);
    board.setCell(0, 3, "wR");
    REQUIRE(RookRule().isLegalMove(Position{0, 3}, Position{0, 0}, board) == true);
}

TEST_CASE("rook cannot jump over a friendly piece in its path") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 2, "wK");
    REQUIRE(RookRule().isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

// ===== BishopRule =====

TEST_CASE("bishop can move diagonally on a clear path") {
    Board board(4, 4);
    board.setCell(0, 0, "wB");
    REQUIRE(BishopRule().isLegalMove(Position{0, 0}, Position{3, 3}, board) == true);
}

TEST_CASE("bishop cannot move in a straight line") {
    Board board(4, 4);
    board.setCell(0, 0, "wB");
    REQUIRE(BishopRule().isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("bishop cannot jump over a piece on the diagonal") {
    Board board(4, 4);
    board.setCell(0, 0, "wB");
    board.setCell(1, 1, "wK");
    REQUIRE(BishopRule().isLegalMove(Position{0, 0}, Position{3, 3}, board) == false);
}

TEST_CASE("bishop can capture an enemy piece directly at the destination") {
    Board board(4, 4);
    board.setCell(0, 0, "wB");
    board.setCell(3, 3, "bK");
    REQUIRE(BishopRule().isLegalMove(Position{0, 0}, Position{3, 3}, board) == true);
}

TEST_CASE("bishop can move along the anti-diagonal") {
    Board board(4, 4);
    board.setCell(0, 3, "wB");
    REQUIRE(BishopRule().isLegalMove(Position{0, 3}, Position{3, 0}, board) == true);
}

TEST_CASE("bishop cannot jump over a piece on the anti-diagonal") {
    Board board(4, 4);
    board.setCell(0, 3, "wB");
    board.setCell(1, 2, "wK");
    REQUIRE(BishopRule().isLegalMove(Position{0, 3}, Position{3, 0}, board) == false);
}

// ===== QueenRule =====

TEST_CASE("queen can move in a straight line") {
    Board board(4, 4);
    board.setCell(0, 0, "wQ");
    REQUIRE(QueenRule().isLegalMove(Position{0, 0}, Position{0, 3}, board) == true);
}

TEST_CASE("queen can move diagonally") {
    Board board(4, 4);
    board.setCell(0, 0, "wQ");
    REQUIRE(QueenRule().isLegalMove(Position{0, 0}, Position{3, 3}, board) == true);
}

TEST_CASE("queen cannot move in an L shape") {
    Board board(4, 4);
    board.setCell(0, 0, "wQ");
    REQUIRE(QueenRule().isLegalMove(Position{0, 0}, Position{2, 1}, board) == false);
}

TEST_CASE("queen cannot jump over a piece in a straight line") {
    Board board(4, 4);
    board.setCell(0, 0, "wQ");
    board.setCell(0, 1, "wK");
    REQUIRE(QueenRule().isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("queen cannot jump over a piece on the diagonal") {
    Board board(4, 4);
    board.setCell(0, 0, "wQ");
    board.setCell(1, 1, "wK");
    REQUIRE(QueenRule().isLegalMove(Position{0, 0}, Position{3, 3}, board) == false);
}

TEST_CASE("queen can move along the anti-diagonal") {
    Board board(4, 4);
    board.setCell(0, 3, "wQ");
    REQUIRE(QueenRule().isLegalMove(Position{0, 3}, Position{3, 0}, board) == true);
}

TEST_CASE("queen can move from right to left") {
    Board board(4, 4);
    board.setCell(0, 3, "wQ");
    REQUIRE(QueenRule().isLegalMove(Position{0, 3}, Position{0, 0}, board) == true);
}

// ===== KingRule =====

TEST_CASE("king can move one cell in any of the eight directions") {
    Board board(3, 3);
    REQUIRE(KingRule().isLegalMove(Position{1, 1}, Position{0, 0}, board) == true);
    REQUIRE(KingRule().isLegalMove(Position{1, 1}, Position{0, 1}, board) == true);
    REQUIRE(KingRule().isLegalMove(Position{1, 1}, Position{0, 2}, board) == true);
    REQUIRE(KingRule().isLegalMove(Position{1, 1}, Position{1, 0}, board) == true);
    REQUIRE(KingRule().isLegalMove(Position{1, 1}, Position{1, 2}, board) == true);
    REQUIRE(KingRule().isLegalMove(Position{1, 1}, Position{2, 0}, board) == true);
    REQUIRE(KingRule().isLegalMove(Position{1, 1}, Position{2, 1}, board) == true);
    REQUIRE(KingRule().isLegalMove(Position{1, 1}, Position{2, 2}, board) == true);
}

TEST_CASE("king cannot move two cells in a straight line") {
    Board board(4, 4);
    REQUIRE(KingRule().isLegalMove(Position{0, 0}, Position{0, 2}, board) == false);
}

TEST_CASE("king cannot move two cells diagonally") {
    Board board(4, 4);
    REQUIRE(KingRule().isLegalMove(Position{0, 0}, Position{2, 2}, board) == false);
}

TEST_CASE("king cannot stay on the same cell") {
    Board board(3, 3);
    REQUIRE(KingRule().isLegalMove(Position{1, 1}, Position{1, 1}, board) == false);
}

// ===== KnightRule =====

TEST_CASE("knight can move in all eight L shapes") {
    Board board(5, 5);
    Position center{2, 2};
    REQUIRE(KnightRule().isLegalMove(center, Position{0, 1}, board) == true);
    REQUIRE(KnightRule().isLegalMove(center, Position{0, 3}, board) == true);
    REQUIRE(KnightRule().isLegalMove(center, Position{1, 0}, board) == true);
    REQUIRE(KnightRule().isLegalMove(center, Position{1, 4}, board) == true);
    REQUIRE(KnightRule().isLegalMove(center, Position{3, 0}, board) == true);
    REQUIRE(KnightRule().isLegalMove(center, Position{3, 4}, board) == true);
    REQUIRE(KnightRule().isLegalMove(center, Position{4, 1}, board) == true);
    REQUIRE(KnightRule().isLegalMove(center, Position{4, 3}, board) == true);
}

TEST_CASE("knight cannot move in a straight line") {
    Board board(5, 5);
    REQUIRE(KnightRule().isLegalMove(Position{2, 2}, Position{2, 4}, board) == false);
}

TEST_CASE("knight cannot move diagonally") {
    Board board(5, 5);
    REQUIRE(KnightRule().isLegalMove(Position{2, 2}, Position{4, 4}, board) == false);
}

TEST_CASE("knight cannot stay on the same cell") {
    Board board(5, 5);
    REQUIRE(KnightRule().isLegalMove(Position{2, 2}, Position{2, 2}, board) == false);
}

TEST_CASE("knight can jump over pieces that would otherwise block a sliding piece") {
    Board board(4, 4);
    board.setCell(0, 0, "wN");
    board.setCell(0, 1, "wK");
    board.setCell(1, 0, "bK");
    REQUIRE(KnightRule().isLegalMove(Position{0, 0}, Position{1, 2}, board) == true);
}

// ===== RulesEngine =====

TEST_CASE("rules engine dispatches rook moves to the rook rule") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{0, 3}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{3, 3}, board) == false);
}

TEST_CASE("rules engine dispatches bishop moves to the bishop rule") {
    Board board(4, 4);
    board.setCell(0, 0, "wB");
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{3, 3}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("rules engine dispatches queen moves to the queen rule") {
    Board board(4, 4);
    board.setCell(0, 0, "wQ");
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{0, 3}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{3, 3}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{2, 1}, board) == false);
}

TEST_CASE("rules engine dispatches king moves to the king rule") {
    Board board(4, 4);
    board.setCell(1, 1, "wK");
    REQUIRE(RulesEngine::isLegalMove(Position{1, 1}, Position{2, 2}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{1, 1}, Position{3, 3}, board) == false);
}

TEST_CASE("rules engine dispatches knight moves to the knight rule") {
    Board board(4, 4);
    board.setCell(0, 0, "wN");
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{1, 2}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("rules engine rejects a move from an empty cell") {
    Board board(4, 4);
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

// ===== PawnRule =====
TEST_CASE("white pawn can move one cell upward to an empty cell") {
    Board board(4, 4);
    board.setCell(2, 1, "wP");
    REQUIRE(PawnRule().isLegalMove(Position{2, 1}, Position{1, 1}, board) == true);
}

TEST_CASE("black pawn can move one cell downward to an empty cell") {
    Board board(4, 4);
    board.setCell(1, 1, "bP");
    REQUIRE(PawnRule().isLegalMove(Position{1, 1}, Position{2, 1}, board) == true);
}

TEST_CASE("white pawn can capture diagonally") {
    Board board(4, 4);
    board.setCell(2, 1, "wP");
    board.setCell(1, 2, "bK");
    REQUIRE(PawnRule().isLegalMove(Position{2, 1}, Position{1, 2}, board) == true);
}

TEST_CASE("black pawn can capture diagonally") {
    Board board(4, 4);
    board.setCell(1, 1, "bP");
    board.setCell(2, 2, "wK");
    REQUIRE(PawnRule().isLegalMove(Position{1, 1}, Position{2, 2}, board) == true);
}

TEST_CASE("pawn cannot move two cells") {
    Board board(4, 4);
    board.setCell(3, 1, "wP");
    REQUIRE(PawnRule().isLegalMove(Position{3, 1}, Position{1, 1}, board) == false);
}

TEST_CASE("pawn cannot move diagonally to an empty cell") {
    Board board(4, 4);
    board.setCell(2, 1, "wP");
    REQUIRE(PawnRule().isLegalMove(Position{2, 1}, Position{1, 2}, board) == false);
}

TEST_CASE("pawn cannot capture a piece directly in front of it") {
    Board board(4, 4);
    board.setCell(2, 1, "wP");
    board.setCell(1, 1, "bK");
    REQUIRE(PawnRule().isLegalMove(Position{2, 1}, Position{1, 1}, board) == false);
}

TEST_CASE("white pawn cannot move backward") {
    Board board(4, 4);
    board.setCell(1, 1, "wP");
    REQUIRE(PawnRule().isLegalMove(Position{1, 1}, Position{2, 1}, board) == false);
}

TEST_CASE("white pawn can capture diagonally to the left") {
    Board board(4, 4);
    board.setCell(2, 1, "wP");
    board.setCell(1, 0, "bK");
    REQUIRE(PawnRule().isLegalMove(Position{2, 1}, Position{1, 0}, board) == true);
}

TEST_CASE("black pawn can capture diagonally to the left") {
    Board board(4, 4);
    board.setCell(1, 1, "bP");
    board.setCell(2, 0, "wK");
    REQUIRE(PawnRule().isLegalMove(Position{1, 1}, Position{2, 0}, board) == true);
}

TEST_CASE("black pawn cannot move diagonally to an empty cell") {
    Board board(4, 4);
    board.setCell(1, 1, "bP");
    REQUIRE(PawnRule().isLegalMove(Position{1, 1}, Position{2, 2}, board) == false);
}

TEST_CASE("black pawn cannot capture a piece directly in front of it") {
    Board board(4, 4);
    board.setCell(1, 1, "bP");
    board.setCell(2, 1, "wK");
    REQUIRE(PawnRule().isLegalMove(Position{1, 1}, Position{2, 1}, board) == false);
}

TEST_CASE("black pawn cannot move backward") {
    Board board(4, 4);
    board.setCell(2, 1, "bP");
    REQUIRE(PawnRule().isLegalMove(Position{2, 1}, Position{1, 1}, board) == false);
}

TEST_CASE("pawn cannot move sideways") {
    Board board(4, 4);
    board.setCell(2, 1, "wP");
    REQUIRE(PawnRule().isLegalMove(Position{2, 1}, Position{2, 2}, board) == false);
}

TEST_CASE("rules engine dispatches pawn moves to the pawn rule") {
    Board board(4, 4);
    board.setCell(2, 1, "wP");
    REQUIRE(RulesEngine::isLegalMove(Position{2, 1}, Position{1, 1}, board) == true);
    REQUIRE(RulesEngine::isLegalMove(Position{2, 1}, Position{0, 1}, board) == false);
}

TEST_CASE("rules engine rejects a pawn capturing diagonally onto its own color") {
    Board board(4, 4);
    board.setCell(2, 1, "wP");
    board.setCell(1, 2, "wK");
    REQUIRE(RulesEngine::isLegalMove(Position{2, 1}, Position{1, 2}, board) == false);
}

TEST_CASE("rules engine rejects capturing a piece of the same color") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 3, "wK");
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{0, 3}, board) == false);
}

TEST_CASE("rules engine allows capturing an enemy piece at the destination") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 3, "bK");
    REQUIRE(RulesEngine::isLegalMove(Position{0, 0}, Position{0, 3}, board) == true);
}
