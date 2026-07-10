#pragma once

enum class Color {
    WHITE,
    BLACK
};

enum class PieceType {
    KING,
    QUEEN,
    ROOK,
    BISHOP,
    KNIGHT,
    PAWN
};

class Piece {
public:
    Piece(Color color, PieceType type) : color_(color), type_(type) {}

    Color getColor() const { return color_; }
    PieceType getType() const { return type_; }

private:
    Color color_;
    PieceType type_;
};