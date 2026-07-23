#include "controler.hpp"
#include "graphics/compositing.hpp"
#include "input/board_mapper.hpp"
#include <filesystem>
#include <tuple>

namespace fs = std::filesystem;

Controler::Controler(const std::string& asset_root, const std::string& theme, const std::string& server_url)
    : board_view((fs::path(asset_root) / "board.png").string()),
      piece_assets((fs::path(asset_root) / theme).string()),
      server_connection_(server_url),
      command_sender_(server_connection_),
      client_board_state_(event_bus_),
      network_receiver_(server_connection_, event_bus_),
      animation_tracker(event_bus_, piece_assets) {
    server_connection_.start();
}

cv::Mat Controler::render_frame() {
    cv::Mat frame = board_view.base().clone();
    Board board = client_board_state_.board();

    for (int row = 0; row < board.getRowCount(); ++row) {
        for (int col = 0; col < board.getColCount(); ++col) {
            std::string token = board.getCell(row, col);
            if (token == Board::EMPTY_CELL) continue;

            std::string code = token_to_piece_code(token);
            AnimationTracker::Status status = animation_tracker.update(row, col, code);

            const cv::Mat& image = piece_assets.current_image_for(code, status.state, status.time_in_state);

            int x, y;
            if (status.move_from.has_value() && status.move_to.has_value()) {
                std::tie(x, y) = board_view.interpolated_pixel(status.move_from->first, status.move_from->second,
                                                                 status.move_to->first, status.move_to->second,
                                                                 status.move_progress);
            } else {
                std::tie(x, y) = board_view.cell_to_pixel(row, col);
            }

            blit_with_alpha(frame, image, x, y);
        }
    }

    return frame;
}

void Controler::run() {
    install_mouse_callback();
    while (true) {
        cv::Mat frame = render_frame();
        cv::imshow(kWindowName, frame);
        int key = cv::waitKey(16);
        if (should_exit(key)) break;
    }
    cv::destroyAllWindows();
}

void Controler::install_mouse_callback() {
    cv::namedWindow(kWindowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(kWindowName, 900, 900);
    cv::setMouseCallback(kWindowName, &Controler::on_mouse_trampoline, this);
}

void Controler::on_mouse_trampoline(int event, int x, int y, int flags, void* userdata) {
    static_cast<Controler*>(userdata)->on_mouse_event(event, x, y);
}

void Controler::on_mouse_event(int event, int x, int y) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        handle_click(x, y);
    } else if (event == cv::EVENT_RBUTTONDOWN) {
        handle_jump(x, y);
    }
}

void Controler::handle_jump(int window_x, int window_y) {
    std::optional<cv::Point> image_point = window_to_image_point(window_x, window_y);
    if (!image_point.has_value()) return;

    auto [row, col] = board_view.pixel_to_cell(image_point->x, image_point->y);
    command_sender_.sendJump(row, col);
}

void Controler::handle_click(int window_x, int window_y) {
    std::optional<cv::Point> image_point = window_to_image_point(window_x, window_y);
    if (!image_point.has_value()) return;
    
    auto [row, col] = board_view.pixel_to_cell(image_point->x, image_point->y);
    command_sender_.sendClick(row, col);
}

std::optional<cv::Point> Controler::window_to_image_point(int window_x, int window_y) const {
    return cv::Point(window_x, window_y);
}
