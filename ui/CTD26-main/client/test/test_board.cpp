#include "test_framework.hpp"
#include "../src/game/graphics/BoardView.hpp"
#include <opencv2/opencv.hpp>
#include <filesystem>

namespace fs = std::filesystem;

namespace {
std::string make_temp_png(int w, int h) {
    fs::path path = fs::temp_directory_path() / "kfc_test_board.png";
    cv::Mat img(h, w, CV_8UC4, cv::Scalar(0, 0, 0, 255));
    cv::imwrite(path.string(), img);
    return path.string();
}
}

TEST(board_loads_and_normalizes_size) {
    BoardView board(make_temp_png(50, 50));
    EXPECT_EQ(board.base().cols, BoardView::kSizePx);
    EXPECT_EQ(board.base().rows, BoardView::kSizePx);
}

TEST(board_cell_to_pixel_corners) {
    BoardView board(make_temp_png(50, 50));
    auto [x0, y0] = board.cell_to_pixel(0, 0);
    EXPECT_EQ(x0, BoardView::kBoardLeftPx);
    EXPECT_EQ(y0, BoardView::kBoardTopPx);

    auto [x7, y7] = board.cell_to_pixel(7, 7);
    EXPECT_EQ(x7, BoardView::kBoardLeftPx + 7 * BoardView::kCellPx);
    EXPECT_EQ(y7, BoardView::kBoardTopPx + 7 * BoardView::kCellPx);
}

TEST(board_throws_on_missing_file) {
    EXPECT_THROWS(BoardView("this/path/does/not/exist.png"));
}

TEST(board_pixel_to_cell_is_the_inverse_of_cell_to_pixel) {
    BoardView board(make_temp_png(50, 50));
    auto [x, y] = board.cell_to_pixel(3, 5);
    auto [row, col] = board.pixel_to_cell(x, y);
    EXPECT_EQ(row, 3);
    EXPECT_EQ(col, 5);
}

TEST(board_interpolated_pixel_at_progress_zero_is_the_from_cell) {
    BoardView board(make_temp_png(50, 50));
    auto [fx, fy] = board.cell_to_pixel(1, 1);
    auto [x, y] = board.interpolated_pixel(1, 1, 4, 4, 0.0);
    EXPECT_EQ(x, fx);
    EXPECT_EQ(y, fy);
}

TEST(board_interpolated_pixel_at_progress_one_is_the_to_cell) {
    BoardView board(make_temp_png(50, 50));
    auto [tx, ty] = board.cell_to_pixel(4, 4);
    auto [x, y] = board.interpolated_pixel(1, 1, 4, 4, 1.0);
    EXPECT_EQ(x, tx);
    EXPECT_EQ(y, ty);
}

TEST(board_interpolated_pixel_at_progress_half_is_the_midpoint) {
    BoardView board(make_temp_png(50, 50));
    auto [x, y] = board.interpolated_pixel(0, 0, 2, 0, 0.5);
    EXPECT_EQ(x, BoardView::kBoardLeftPx);
    EXPECT_EQ(y, BoardView::kBoardTopPx + BoardView::kCellPx);
}
