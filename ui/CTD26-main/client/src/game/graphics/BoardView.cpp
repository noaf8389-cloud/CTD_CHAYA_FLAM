#include "BoardView.hpp"
#include "../../img.hpp"

BoardView::BoardView(const std::string& board_png_path) {
    Img loader;
    loader.read(board_png_path);
    board_mat = loader.get_mat().clone();
    cv::resize(board_mat, board_mat, cv::Size(kSizePx, kSizePx));
}

std::pair<int, int> BoardView::interpolated_pixel(int from_row, int from_col, int to_row, int to_col, double progress) const {
    auto [from_x, from_y] = cell_to_pixel(from_row, from_col);
    auto [to_x, to_y] = cell_to_pixel(to_row, to_col);
    int x = static_cast<int>(from_x + (to_x - from_x) * progress);
    int y = static_cast<int>(from_y + (to_y - from_y) * progress);
    return {x, y};
}
