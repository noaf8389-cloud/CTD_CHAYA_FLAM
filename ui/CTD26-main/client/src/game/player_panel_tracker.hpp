#pragma once

#include <optional>

#include "player_panel_data.hpp"
#include "model/Position.hpp"

class EventBus;

// Listens to score/move events on the bus and maintains display data for both players' panels.
class PlayerPanelTracker {
public:
    explicit PlayerPanelTracker(EventBus& bus);

    const PlayerPanelData& black() const { return black_; }
    const PlayerPanelData& white() const { return white_; }

private:
    struct PendingMove {
        Position to;
        char color;
        bool isPawn;
        int fromCol;
    };

    PlayerPanelData black_;
    PlayerPanelData white_;
    int rowCount_ = 8;
    std::optional<PendingMove> pendingMove_;
};
