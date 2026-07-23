#include "test_framework.hpp"
#include "../src/game/animation_tracker.hpp"
#include "../src/game/graphics/piece_assets.hpp"
#include "../src/game/bus/event_bus.hpp"
#include <filesystem>
#include <fstream>
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

namespace {
fs::path make_theme(const std::string& name) {
    fs::path root = fs::temp_directory_path() / name;
    fs::remove_all(root);
    fs::create_directories(root);
    return root;
}

void write_state(const fs::path& theme_root, const std::string& code, const std::string& state,
                  double fps, const std::string& next_state) {
    fs::path state_dir = theme_root / code / "states" / state;
    fs::path sprites_dir = state_dir / "sprites";
    fs::create_directories(sprites_dir);
    cv::imwrite((sprites_dir / "1.png").string(), cv::Mat(10, 10, CV_8UC3, cv::Scalar(0, 0, 0)));

    std::ofstream config(state_dir / "config.json");
    config << "{\"graphics\": {\"frames_per_sec\": " << fps << ", \"is_loop\": true},"
           << " \"physics\": {\"next_state_when_finished\": \"" << next_state << "\"}}";
}

// Each mode: 1 frame, 10 fps -> 0.1 second cycle. move -> long_rest -> idle. jump -> short_rest -> idle.
PieceAssets make_test_assets() {
    fs::path theme = make_theme("kfc_test_theme_animation_tracker");
    write_state(theme, "AA", "move", 10.0, "long_rest");
    write_state(theme, "AA", "jump", 10.0, "short_rest");
    write_state(theme, "AA", "long_rest", 10.0, "idle");
    write_state(theme, "AA", "short_rest", 10.0, "idle");
    write_state(theme, "AA", "idle", 10.0, "idle");
    return PieceAssets(theme.string());
}
}

TEST(animation_tracker_defaults_new_position_to_idle) {
    PieceAssets assets = make_test_assets();
    EventBus bus;
    double fake_now = 5.0;
    AnimationTracker tracker(bus, assets, [&fake_now]() { return fake_now; });

    auto status = tracker.update(0, 0, "AA");
    EXPECT_EQ(status.state, std::string("idle"));
    EXPECT_EQ(status.time_in_state, 0.0);
}

TEST(animation_tracker_enters_move_state_on_move_started_event) {
    PieceAssets assets = make_test_assets();
    EventBus bus;
    double fake_now = 5.0;
    AnimationTracker tracker(bus, assets, [&fake_now]() { return fake_now; });

    bus.publish(MoveStartedEvent{Position{0, 0}, Position{0, 3}, "wP", 300, 5000});

    auto status = tracker.update(0, 0, "AA");
    EXPECT_EQ(status.state, std::string("move"));
    EXPECT_EQ(status.time_in_state, 0.0);
}

TEST(animation_tracker_prefers_jump_signal) {
    PieceAssets assets = make_test_assets();
    EventBus bus;
    double fake_now = 3.0;
    AnimationTracker tracker(bus, assets, [&fake_now]() { return fake_now; });

    bus.publish(JumpStartedEvent{Position{0, 0}, "aA", 500, 3000});

    auto status = tracker.update(0, 0, "AA");
    EXPECT_EQ(status.state, std::string("jump"));
}

TEST(animation_tracker_keeps_timer_while_move_persists) {
    PieceAssets assets = make_test_assets();
    EventBus bus;
    double fake_now = 5.0;
    AnimationTracker tracker(bus, assets, [&fake_now]() { return fake_now; });

    bus.publish(MoveStartedEvent{Position{0, 0}, Position{0, 3}, "wP", 1000, 5000});
    fake_now = 5.3;
    auto status = tracker.update(0, 0, "AA");

    EXPECT_EQ(status.state, std::string("move"));
    EXPECT_TRUE(status.time_in_state > 0.29 && status.time_in_state < 0.31);
}

TEST(animation_tracker_relocates_to_destination_on_move_made_event) {
    PieceAssets assets = make_test_assets();
    EventBus bus;
    double fake_now = 1.0;
    AnimationTracker tracker(bus, assets, [&fake_now]() { return fake_now; });

    bus.publish(MoveStartedEvent{Position{0, 0}, Position{0, 3}, "aA", 200, 1000});
    fake_now = 1.2;
    bus.publish(MoveMadeEvent{Position{0, 0}, Position{0, 3}, "aA", 1200});

    auto status = tracker.update(0, 3, "AA");
    EXPECT_EQ(status.state, std::string("long_rest"));
    EXPECT_EQ(status.time_in_state, 0.0);
}

TEST(animation_tracker_does_not_relocate_before_move_made_event) {
    PieceAssets assets = make_test_assets();
    EventBus bus;
    double fake_now = 1.0;
    AnimationTracker tracker(bus, assets, [&fake_now]() { return fake_now; });

    bus.publish(MoveStartedEvent{Position{0, 0}, Position{0, 3}, "wP", 1000, 1000});
    fake_now = 1.1;

    auto status = tracker.update(0, 0, "AA");
    EXPECT_EQ(status.state, std::string("move"));   //  MoveMadeEvent still don't come
}

TEST(animation_tracker_transitions_to_short_rest_when_jump_lands) {
    PieceAssets assets = make_test_assets();
    EventBus bus;
    double fake_now = 1.0;
    AnimationTracker tracker(bus, assets, [&fake_now]() { return fake_now; });

    bus.publish(JumpStartedEvent{Position{0, 0}, "aA", 200, 1000});
    fake_now = 1.2;
    bus.publish(JumpLandedEvent{Position{0, 0}, "aA", 1200});

    auto status = tracker.update(0, 0, "AA");
    EXPECT_EQ(status.state, std::string("short_rest"));
}

TEST(animation_tracker_stays_in_rest_regardless_of_elapsed_time) {
    PieceAssets assets = make_test_assets();
    EventBus bus;
    double fake_now = 1.0;
    AnimationTracker tracker(bus, assets, [&fake_now]() { return fake_now; });

    bus.publish(JumpStartedEvent{Position{0, 0}, "aA", 50, 1000});
    fake_now = 1.05;
    bus.publish(JumpLandedEvent{Position{0, 0}, "aA", 1050});  // -> short_rest, time=0

    fake_now = 5.0;   // Much beyond the rest animation cycle
    auto status = tracker.update(0, 0, "AA");
    EXPECT_EQ(status.state, std::string("short_rest"));   // Not progressing on its own — waiting for RestEndedEvent
}

TEST(animation_tracker_transitions_to_idle_on_rest_ended_event) {
    PieceAssets assets = make_test_assets();
    EventBus bus;
    double fake_now = 1.0;
    AnimationTracker tracker(bus, assets, [&fake_now]() { return fake_now; });

    bus.publish(JumpStartedEvent{Position{0, 0}, "aA", 50, 1000});
    fake_now = 1.05;
    bus.publish(JumpLandedEvent{Position{0, 0}, "aA", 1050});  // -> short_rest, time=0

    fake_now = 4.0;
    bus.publish(RestEndedEvent{Position{0, 0}, "aA", 4000});

    auto status = tracker.update(0, 0, "AA");
    EXPECT_EQ(status.state, std::string("idle"));
}

TEST(animation_tracker_tracks_positions_independently) {
    PieceAssets assets = make_test_assets();
    EventBus bus;
    double fake_now = 1.0;
    AnimationTracker tracker(bus, assets, [&fake_now]() { return fake_now; });

    bus.publish(MoveStartedEvent{Position{0, 0}, Position{0, 3}, "wP", 500, 1000});
    auto status_b = tracker.update(3, 3, "AA");

    EXPECT_EQ(status_b.state, std::string("idle"));   // Not affected by the other slot
}

TEST(animation_tracker_rest_ended_event_for_unknown_position_creates_a_fresh_entry) {
    PieceAssets assets = make_test_assets();
    EventBus bus;
    double fake_now = 2.0;
    AnimationTracker tracker(bus, assets, [&fake_now]() { return fake_now; });

    bus.publish(RestEndedEvent{Position{2, 2}, "aA", 2000});

    auto status = tracker.update(2, 2, "AA");
    EXPECT_EQ(status.state, std::string("idle"));
}

TEST(animation_tracker_rest_ended_event_does_not_affect_a_different_position) {
    PieceAssets assets = make_test_assets();
    EventBus bus;
    double fake_now = 1.0;
    AnimationTracker tracker(bus, assets, [&fake_now]() { return fake_now; });

    bus.publish(JumpStartedEvent{Position{0, 0}, "aA", 50, 1000});
    fake_now = 1.05;
    bus.publish(JumpLandedEvent{Position{0, 0}, "aA", 1050});

    bus.publish(RestEndedEvent{Position{3, 3}, "aA", 1050});

    auto status = tracker.update(0, 0, "AA");
    EXPECT_EQ(status.state, std::string("short_rest"));   // Not affected by a rest that ended in a different slot
}
