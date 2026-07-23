#pragma once
#include <functional>

struct Position {
    int row;
    int col;
};

inline bool operator==(const Position& lhs, const Position& rhs) {
    return lhs.row == rhs.row && lhs.col == rhs.col;
}