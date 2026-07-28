#pragma once
#include <vector>
#include <optional>
#include <string>

class Board {
public:
    static inline const std::string EMPTY_CELL = ".";

    Board(int rows, int cols) : rows_(rows), cols_(cols), cells_(rows * cols, EMPTY_CELL) {}

    int getRowCount() const { return rows_; }
    int getColCount() const { return cols_; }

    std::string getCell(int row, int col) const { return cells_.at(toIndex(row, col)); }
    void setCell(int row, int col, const std::string& token) { cells_.at(toIndex(row, col)) = token; }


private:
    int rows_;
    int cols_;
    std::vector<std::string> cells_;

    int toIndex(int row, int col) const { return row * cols_ + col; }
};