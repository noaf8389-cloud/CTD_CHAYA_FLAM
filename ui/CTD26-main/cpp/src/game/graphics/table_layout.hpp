#pragma once

#include <opencv2/opencv.hpp>
#include "../player_panel_data.hpp"

// Draws the board inside a larger "table" frame: name plaques above/below the board,
// move-log panels on both sides. Pure rendering — knows nothing about game logic or networking.
class TableLayout {
public:
    TableLayout(int boardWidth, int boardHeight);

    cv::Size canvasSize() const { return canvas_size_; }
    cv::Point boardOffset() const { return board_offset_; }

    cv::Mat render(const cv::Mat& boardImage, const PlayerPanelData& blackPlayer, const PlayerPanelData& whitePlayer) const;

private:
    void drawPlaque(cv::Mat& canvas, const cv::Rect& rect, const PlayerPanelData& player) const;
    void drawPanel(cv::Mat& canvas, const cv::Rect& rect, const PlayerPanelData& player) const;

    int board_width_;
    int board_height_;
    cv::Size canvas_size_;
    cv::Point board_offset_;

    static constexpr int kPanelWidth = 230;
    static constexpr int kPlaqueHeight = 64;
    static constexpr int kMargin = 16;
};
