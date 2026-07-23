#include "animation_tracker.hpp"

#include <algorithm>

#include "bus/event_bus.hpp"
#include "piece_code.hpp"

AnimationTracker::Clock AnimationTracker::real_clock() {
    auto start = std::chrono::steady_clock::now();
    return [start]() {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    };
}

AnimationTracker::AnimationTracker(EventBus& bus, const PieceAssets& piece_assets, Clock clock)
    : clock_(std::move(clock)), piece_assets_(piece_assets) {
    bus.subscribe<MoveStartedEvent>([this](const MoveStartedEvent& e) { onMoveStarted(e); });
    bus.subscribe<MoveMadeEvent>([this](const MoveMadeEvent& e) { onMoveMade(e); });
    bus.subscribe<JumpStartedEvent>([this](const JumpStartedEvent& e) { onJumpStarted(e); });
    bus.subscribe<JumpLandedEvent>([this](const JumpLandedEvent& e) { onJumpLanded(e); });
    bus.subscribe<RestEndedEvent>([this](const RestEndedEvent& e) { onRestEnded(e); });
}

void AnimationTracker::onMoveStarted(const MoveStartedEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::pair<int, int> key{event.from.row, event.from.col};
    entries_[key] = Entry{
        "move",
        clock_(),
        std::make_pair(event.from.row, event.from.col),
        std::make_pair(event.to.row, event.to.col),
        event.duration_ms / 1000.0
    };
}

void AnimationTracker::onMoveMade(const MoveMadeEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::pair<int, int> from_key{event.from.row, event.from.col};
    std::pair<int, int> to_key{event.to.row, event.to.col};

    std::string code = token_to_piece_code(event.piece_token);
    entries_.erase(from_key);
    entries_[to_key] = Entry{piece_assets_.next_state_for(code, "move"), clock_(), std::nullopt, std::nullopt, 0.0};
}

void AnimationTracker::onJumpStarted(const JumpStartedEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::pair<int, int> key{event.position.row, event.position.col};
    entries_[key] = Entry{"jump", clock_(), std::nullopt, std::nullopt, 0.0};
}

void AnimationTracker::onJumpLanded(const JumpLandedEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::pair<int, int> key{event.position.row, event.position.col};
    std::string code = token_to_piece_code(event.piece_token);
    entries_[key] = Entry{piece_assets_.next_state_for(code, "jump"), clock_(), std::nullopt, std::nullopt, 0.0};
}

void AnimationTracker::onRestEnded(const RestEndedEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::pair<int, int> key{event.position.row, event.position.col};
    std::string code = token_to_piece_code(event.piece_token);
    entries_[key] = Entry{piece_assets_.next_state_for(code, "rest"), clock_(), std::nullopt, std::nullopt, 0.0};
}

AnimationTracker::Status AnimationTracker::update(int row, int col, const std::string& code) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::pair<int, int> key{row, col};
    double now = clock_();
    auto it = entries_.find(key);

    if (it == entries_.end()) {
        entries_[key] = Entry{"idle", now, std::nullopt, std::nullopt, 0.0};
    } else if (it->second.state != "move" && it->second.state != "jump" && it->second.state != "rest"
               && piece_assets_.is_state_finished(code, it->second.state, now - it->second.state_entered_at)) {
        entries_[key] = Entry{piece_assets_.next_state_for(code, it->second.state), now, std::nullopt, std::nullopt, 0.0};
    }

    const Entry& entry = entries_[key];
    double time_in_state = now - entry.state_entered_at;

    double progress = 0.0;
    if (entry.move_from.has_value()) {
        progress = entry.move_duration_seconds > 0.0
            ? std::clamp(time_in_state / entry.move_duration_seconds, 0.0, 1.0)
            : 1.0;
    }

    return Status{entry.state, time_in_state, entry.move_from, entry.move_to, progress};
}