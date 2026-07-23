#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

#include "graphics/piece_assets.hpp"
#include "bus/game_events.hpp"

class EventBus;

class AnimationTracker {
public:
    struct Status {
        std::string state;
        double time_in_state;
        std::optional<std::pair<int, int>> move_from;
        std::optional<std::pair<int, int>> move_to;
        double move_progress = 0.0;
    };

    using Clock = std::function<double()>;

    // Subscribes to move-related events to drive move animation independently of per-frame polling.
    // clock supplies the current time in seconds; defaults to real elapsed time, overridable in tests.
    AnimationTracker(EventBus& bus, const PieceAssets& piece_assets, Clock clock = real_clock());

    // Determines (and records) which animation state a piece at a given cell should currently be displaying.
    Status update(int row, int col, const std::string& code);
    
private:
    struct Entry {
        std::string state;
        double state_entered_at;
        std::optional<std::pair<int, int>> move_from;
        std::optional<std::pair<int, int>> move_to;
        double move_duration_seconds = 0.0;
    };

    static Clock real_clock();
    void onMoveStarted(const MoveStartedEvent& event);
    void onMoveMade(const MoveMadeEvent& event);
    void onJumpStarted(const JumpStartedEvent& event);
    void onJumpLanded(const JumpLandedEvent& event);
    void onRestEnded(const RestEndedEvent& event);

    std::mutex mutex_;
    Clock clock_;
    const PieceAssets& piece_assets_;
    std::map<std::pair<int, int>, Entry> entries_;
};
