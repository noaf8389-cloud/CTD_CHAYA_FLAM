#include "controler.hpp"
#include "graphics/compositing.hpp"
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <tuple>

namespace fs = std::filesystem;

Controler::Controler(const std::string& asset_root, const std::string& theme, const std::string& username, const std::string& password,
                      MatchChoice matchChoice, const std::string& server_url)
    : match_choice_(std::move(matchChoice)),
      board_view((fs::path(asset_root) / "board.png").string()),
      piece_assets((fs::path(asset_root) / theme).string()),
      table_layout_(board_view.base().cols, board_view.base().rows),
      server_connection_(server_url),
      command_sender_(server_connection_),
      client_board_state_(event_bus_),
      player_panels_(event_bus_),
      network_receiver_(server_connection_, event_bus_),
      animation_tracker(event_bus_, piece_assets) {
    event_bus_.subscribe<LoginResultReceived>([this](const LoginResultReceived& e) {
        if (e.success) {
            send_match_intent();
        } else {
            MessageBoxA(nullptr, "Invalid username or password.", "KungFu Chess", MB_OK | MB_ICONERROR);
            std::exit(1);
        }
    });
    event_bus_.subscribe<RoomCreatedReceived>([](const RoomCreatedReceived& e) {
        MessageBoxA(nullptr, ("Your room code: " + e.roomCode).c_str(), "KungFu Chess", MB_OK | MB_ICONINFORMATION);
    });
    event_bus_.subscribe<JoinRoomFailedReceived>([](const JoinRoomFailedReceived&) {
        MessageBoxA(nullptr, "That room code wasn't found.", "KungFu Chess", MB_OK | MB_ICONWARNING);
    });
    event_bus_.subscribe<NoMatchFoundReceived>([](const NoMatchFoundReceived&) {
        MessageBoxA(nullptr, "No match found. Try again.", "KungFu Chess", MB_OK | MB_ICONWARNING);
    });

    server_connection_.setOnOpen([this, username, password]() { command_sender_.sendLogin(username, password); });
    server_connection_.start();
}

cv::Mat Controler::render_frame() {
    cv::Mat board_frame = board_view.base().clone();
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

            blit_with_alpha(board_frame, image, x, y);
        }
    }

    cv::Mat frame = table_layout_.render(board_frame, player_panels_.black(), player_panels_.white());

    if (client_board_state_.is_game_over()) {
        std::string winner = (client_board_state_.winner_color() == 'w') ? "White" : "Black";
        std::string text = "Game Over - " + winner + " wins!";

        int baseline = 0;
        double fontScale = 1.2;
        int thickness = 2;
        cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline);
        cv::Point textOrigin((frame.cols - textSize.width) / 2, frame.rows / 2);

        cv::Rect banner(textOrigin.x - 20, textOrigin.y - textSize.height - 20, textSize.width + 40, textSize.height + 40);
        cv::rectangle(frame, banner, cv::Scalar(20, 20, 20), cv::FILLED);
        cv::putText(frame, text, textOrigin, cv::FONT_HERSHEY_SIMPLEX, fontScale, cv::Scalar(255, 255, 255), thickness);
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

    cv::Size canvas = table_layout_.canvasSize();

    RECT work_area;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);
    int available_width = work_area.right - work_area.left;
    int available_height = work_area.bottom - work_area.top;

    double scale = std::min({1.0,
        available_width * 0.9 / canvas.width,
        available_height * 0.9 / canvas.height});

    int window_width = static_cast<int>(canvas.width * scale);
    int window_height = static_cast<int>(canvas.height * scale);

    cv::resizeWindow(kWindowName, window_width, window_height);
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

void Controler::send_match_intent() {
    switch (match_choice_.intent) {
        case MatchIntent::CreateRoom: command_sender_.sendCreateRoom(); break;
        case MatchIntent::JoinRoom:   command_sender_.sendJoinRoom(*match_choice_.roomCode); break;
        case MatchIntent::FindMatch:  command_sender_.sendFindMatch(); break;
    }
}

std::optional<cv::Point> Controler::window_to_image_point(int window_x, int window_y) const {
    cv::Point offset = table_layout_.boardOffset();
    cv::Rect boardRect(offset, board_view.base().size());
    cv::Point clicked(window_x, window_y);

    if (!boardRect.contains(clicked)) {
        return std::nullopt;
    }
    return clicked - offset;
}
