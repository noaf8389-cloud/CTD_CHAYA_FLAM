#include "player_registry.hpp"

PlayerRegistry::PlayerRegistry(std::vector<char> colors) : availableColors_(std::move(colors)) {}

PlayerId PlayerRegistry::registerConnection() {
    PlayerId id = nextId_++;
    if (!availableColors_.empty()) {
        assignments_[id] = availableColors_.front();
        availableColors_.erase(availableColors_.begin());
    }
    return id;
}

void PlayerRegistry::unregisterConnection(PlayerId id) {
    auto it = assignments_.find(id);
    if (it != assignments_.end()) {
        availableColors_.push_back(it->second);
        assignments_.erase(it);
    }
}

std::optional<char> PlayerRegistry::colorFor(PlayerId id) const {
    auto it = assignments_.find(id);
    if (it == assignments_.end()) {
        return std::nullopt;
    }
    return it->second;
}
