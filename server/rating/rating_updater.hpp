#pragma once

class EventBus;
class PlayerRegistry;
class PlayerAccountStore;
struct GameOverEvent;

class RatingUpdater {
public:
    // Subscribes to GameOverEvent; on each game end, looks up both players' usernames and ratings and persists the updated ELO scores.
    RatingUpdater(EventBus& bus, PlayerRegistry& registry, PlayerAccountStore& accounts);

private:
    void onGameOver(const GameOverEvent& event);

    PlayerRegistry& registry_;
    PlayerAccountStore& accounts_;
};
