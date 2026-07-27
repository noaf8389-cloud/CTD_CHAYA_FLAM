#include "player_registry.hpp"

PlayerRegistry::PlayerRegistry(std::vector<char> colors) : availableColors_(std::move(colors)) {}

PlayerId PlayerRegistry::registerConnection() {
    PlayerId id = nextId_++;
    if (!availableColors_.empty()) {
        assignments_[id] = PlayerInfo{availableColors_.front(), ""};
        availableColors_.erase(availableColors_.begin());
    }
    return id;
}

void PlayerRegistry::unregisterConnection(PlayerId id) {
    auto it = assignments_.find(id);
    if (it != assignments_.end()) {
        availableColors_.push_back(it->second.color);
        assignments_.erase(it);
    }
}

std::optional<char> PlayerRegistry::colorFor(PlayerId id) const {
    auto it = assignments_.find(id);
    if (it == assignments_.end()) {
        return std::nullopt;
    }
    return it->second.color;
}

void PlayerRegistry::setUsername(PlayerId id, std::string username) {
    auto it = assignments_.find(id);
    if (it != assignments_.end()) {
        it->second.username = std::move(username);
    }
}

std::optional<std::string> PlayerRegistry::usernameFor(PlayerId id) const {
    auto it = assignments_.find(id);
    if (it == assignments_.end() || it->second.username.empty()) {
        return std::nullopt;
    }
    return it->second.username;
}

std::optional<std::string> PlayerRegistry::usernameForColor(char color) const {
    for (const auto& [id, info] : assignments_) {
        if (info.color == color && !info.username.empty()) {
            return info.username;
        }
    }
    return std::nullopt;
}
