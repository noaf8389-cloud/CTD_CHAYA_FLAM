#include "table_layout.hpp"

namespace {
    const cv::Scalar kBackground(17, 22, 30, 255);
    const cv::Scalar kPanelFill(26, 36, 50, 255);
    const cv::Scalar kBrass(50, 150, 195, 255);
    const cv::Scalar kParchment(196, 220, 232, 255);
}

TableLayout::TableLayout(int boardWidth, int boardHeight)
    : board_width_(boardWidth), board_height_(boardHeight) {
    int canvasW = board_width_ + 2 * (kPanelWidth + kMargin);
    int canvasH = board_height_ + 2 * (kPlaqueHeight + kMargin);
    canvas_size_ = cv::Size(canvasW, canvasH);
    board_offset_ = cv::Point(kPanelWidth + kMargin, kPlaqueHeight + kMargin);
}

cv::Mat TableLayout::render(const cv::Mat& boardImage, const PlayerPanelData& blackPlayer, const PlayerPanelData& whitePlayer) const {
    cv::Mat canvas(canvas_size_, boardImage.type(), kBackground);

    cv::Rect boardRect(board_offset_, cv::Size(board_width_, board_height_));
    boardImage.copyTo(canvas(boardRect));
    cv::rectangle(canvas, boardRect, kBrass, 2);

    cv::Rect topPlaque(board_offset_.x, kMargin / 2, board_width_, kPlaqueHeight - kMargin / 2);
    cv::Rect bottomPlaque(board_offset_.x, board_offset_.y + board_height_ + kMargin / 2, board_width_, kPlaqueHeight - kMargin / 2);
    drawPlaque(canvas, topPlaque, blackPlayer);
    drawPlaque(canvas, bottomPlaque, whitePlayer);

    cv::Rect leftPanel(kMargin, board_offset_.y, kPanelWidth, board_height_);
    cv::Rect rightPanel(board_offset_.x + board_width_ + kMargin, board_offset_.y, kPanelWidth, board_height_);
    drawPanel(canvas, leftPanel, blackPlayer);
    drawPanel(canvas, rightPanel, whitePlayer);

    return canvas;
}

void TableLayout::drawPlaque(cv::Mat& canvas, const cv::Rect& rect, const PlayerPanelData& player) const {
    cv::rectangle(canvas, rect, kPanelFill, cv::FILLED);
    cv::rectangle(canvas, rect, kBrass, 1);

    std::string line = player.username + "   (" + std::to_string(player.rating) + ")";
    cv::putText(canvas, line, cv::Point(rect.x + 14, rect.y + rect.height / 2 + 6),
                cv::FONT_HERSHEY_SIMPLEX, 0.65, kParchment, 1, cv::LINE_AA);
}

void TableLayout::drawPanel(cv::Mat& canvas, const cv::Rect& rect, const PlayerPanelData& player) const {
    cv::rectangle(canvas, rect, kPanelFill, cv::FILLED);
    cv::rectangle(canvas, rect, kBrass, 1);

    cv::putText(canvas, "MOVE LOG", cv::Point(rect.x + 12, rect.y + 24),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, kParchment, 1, cv::LINE_AA);

    const int lineHeight = 22;
    const int maxLines = (rect.height - 90) / lineHeight;
    const size_t startIndex = player.moves.size() > static_cast<size_t>(maxLines) ? player.moves.size() - maxLines : 0;

    int y = rect.y + 50;
    for (size_t i = startIndex; i < player.moves.size(); ++i) {
        cv::putText(canvas, player.moves[i], cv::Point(rect.x + 12, y),
                    cv::FONT_HERSHEY_SIMPLEX, 0.45, kParchment, 1, cv::LINE_AA);
        y += lineHeight;
    }

    cv::putText(canvas, "SCORE", cv::Point(rect.x + 12, rect.y + rect.height - 34),
                cv::FONT_HERSHEY_SIMPLEX, 0.45, kParchment, 1, cv::LINE_AA);
    cv::putText(canvas, std::to_string(player.score), cv::Point(rect.x + 12, rect.y + rect.height - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.9, kBrass, 2, cv::LINE_AA);
}
