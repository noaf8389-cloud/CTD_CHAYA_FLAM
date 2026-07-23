#pragma once
#include "animation_tracker.hpp"
#include "graphics/BoardView.hpp"
#include "graphics/piece_assets.hpp"
#include "network/server_connection.hpp"
#include "network/command_sender.hpp"
#include "network/network_receiver.hpp"
#include "bus/event_bus.hpp"
#include "client_board_state.hpp"
#include "piece_code.hpp"
#include <chrono>
#include <optional>
#include <string>


class Controler {
public:
    // Loads piece assets and connects to the server at server_url; the board itself arrives via GameStartedEvent.
    Controler(const std::string& asset_root, const std::string& theme, const std::string& server_url = "ws://127.0.0.1:8080");
    void run();


private:
    static constexpr const char* kWindowName = "KungFu Chess";
    static constexpr int kEscKey = 27;

    cv::Mat render_frame();
    bool should_exit(int key) const { return key == kEscKey || cv::getWindowProperty(kWindowName, cv::WND_PROP_VISIBLE) < 1; }

    void install_mouse_callback();
    static void on_mouse_trampoline(int event, int x, int y, int flags, void* userdata);
    void on_mouse_event(int event, int x, int y);
    std::optional<cv::Point> window_to_image_point(int window_x, int window_y) const;
    void handle_click(int window_x, int window_y);
    void handle_jump(int window_x, int window_y);

    BoardView board_view;
    PieceAssets piece_assets;
    ServerConnection server_connection_;
    CommandSender command_sender_;
    EventBus event_bus_;
    ClientBoardState client_board_state_;
    NetworkReceiver network_receiver_;
    AnimationTracker animation_tracker;
};
