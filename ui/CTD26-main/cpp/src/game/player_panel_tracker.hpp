#pragma once

#include "player_panel_data.hpp"

class EventBus;

// Listens to score/move events on the bus and maintains display data for both players' panels.
class PlayerPanelTracker {
public:
    explicit PlayerPanelTracker(EventBus& bus);

    const PlayerPanelData& black() const { return black_; }
    const PlayerPanelData& white() const { return white_; }

private:
    PlayerPanelData black_;
    PlayerPanelData white_;
};
