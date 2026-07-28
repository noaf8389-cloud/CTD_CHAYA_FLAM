#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <utility>

class BoardView {
    public:
    static constexpr int kSizePx = 1254;
    static constexpr int kBoardCells = 8;
    static constexpr int kCellPx = 122;
    static constexpr int kBoardLeftPx = 143;
    static constexpr int kBoardTopPx = 134;
    
    // Loads the board background image from the given path.
    explicit BoardView(const std::string& board_png_path);

    const cv::Mat& base() const { return board_mat; }

    std::pair<int, int> cell_to_pixel(int row, int col) const {
        return {kBoardLeftPx + col * kCellPx, kBoardTopPx + row * kCellPx};
    }

    std::pair<int, int> pixel_to_cell(int x, int y) const {
        return {(y - kBoardTopPx) / kCellPx, (x - kBoardLeftPx) / kCellPx};
    }

    // Returns the pixel position along the path from one cell to another at the given progress fraction (0..1).
    std::pair<int, int> interpolated_pixel(int from_row, int from_col, int to_row, int to_col, double progress) const;

private:
    cv::Mat board_mat;
};
