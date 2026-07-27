#include "rating_updater.hpp"
#include "elo_calculator.hpp"

#include "../bus/event_bus.hpp"
#include "../bus/game_events.hpp"
#include "../player_registry.hpp"
#include "../db/player_account_store.hpp"

RatingUpdater::RatingUpdater(EventBus& bus, PlayerRegistry& registry, PlayerAccountStore& accounts)
    : registry_(registry), accounts_(accounts) {
    bus.subscribe<GameOverEvent>([this](const GameOverEvent& event) { onGameOver(event); });
}

void RatingUpdater::onGameOver(const GameOverEvent& event) {
    char loserColor = (event.winner_color == 'w') ? 'b' : 'w';

    std::optional<std::string> winnerName = registry_.usernameForColor(event.winner_color);
    std::optional<std::string> loserName = registry_.usernameForColor(loserColor);
    if (!winnerName.has_value() || !loserName.has_value()) {
        return;
    }

    std::optional<int> winnerRating = accounts_.ratingFor(*winnerName);
    std::optional<int> loserRating = accounts_.ratingFor(*loserName);
    if (!winnerRating.has_value() || !loserRating.has_value()) {
        return;
    }

    EloResult result = calculateElo(*winnerRating, *loserRating);
    accounts_.updateRating(*winnerName, result.newWinnerRating);
    accounts_.updateRating(*loserName, result.newLoserRating);
}
