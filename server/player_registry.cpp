#include "player_registry.hpp"

PlayerRegistry::PlayerRegistry(std::vector<char> colors) : availableColors_(std::move(colors)) {}

PlayerId PlayerRegistry::registerConnection() {
    std::lock_guard<std::mutex> lock(mutex_);
    PlayerId id = nextId_++;
    if (!availableColors_.empty()) {
        assignments_[id] = PlayerInfo{availableColors_.front(), ""};
        availableColors_.erase(availableColors_.begin());
    }
    return id;
}

void PlayerRegistry::unregisterConnection(PlayerId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = assignments_.find(id);
    if (it != assignments_.end()) {
        availableColors_.push_back(it->second.color);
        assignments_.erase(it);
    }
}

std::optional<char> PlayerRegistry::colorFor(PlayerId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = assignments_.find(id);
    if (it == assignments_.end()) {
        return std::nullopt;
    }
    return it->second.color;
}

void PlayerRegistry::setUsername(PlayerId id, std::string username) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = assignments_.find(id);
    if (it != assignments_.end()) {
        it->second.username = std::move(username);
    }
}

std::optional<std::string> PlayerRegistry::usernameFor(PlayerId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = assignments_.find(id);
    if (it == assignments_.end() || it->second.username.empty()) {
        return std::nullopt;
    }
    return it->second.username;
}

std::optional<std::string> PlayerRegistry::usernameForColor(char color) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [id, info] : assignments_) {
        if (info.color == color && !info.username.empty()) {
            return info.username;
        }
    }
    return std::nullopt;
}

void PlayerRegistry::beginDisconnectGrace(PlayerId id, long long graceDurationMs) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = assignments_.find(id);
    if (it == assignments_.end()) {
        return;
    }
    if (it->second.username.empty()) {
        availableColors_.push_back(it->second.color);
        assignments_.erase(it);
        return;
    }
    char color = it->second.color;
    pendingDisconnects_.push_back(PendingDisconnect{
        color,
        it->second.username,
        std::chrono::steady_clock::now() + std::chrono::milliseconds(graceDurationMs)
    });
    assignments_.erase(it);
    if (onDisconnect_) {
        onDisconnect_(color, graceDurationMs);
    }
}

bool PlayerRegistry::tryResumeSession(PlayerId newId, const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (size_t i = 0; i < pendingDisconnects_.size(); ++i) {
        if (pendingDisconnects_[i].username == username) {
            char resumedColor = pendingDisconnects_[i].color;
            pendingDisconnects_.erase(pendingDisconnects_.begin() + i);

            // If registerConnection() already handed this new connection a fresh color
            // (e.g. it drew from the pool before the login arrived), return it to the
            // pool instead of leaking it, then restore the resumed color instead.
            auto existing = assignments_.find(newId);
            if (existing != assignments_.end()) {
                availableColors_.push_back(existing->second.color);
            }
            assignments_[newId] = PlayerInfo{resumedColor, username};
            if (onReconnect_) {
                onReconnect_(resumedColor);
            }
            return true;
        }
    }
    return false;
}

void PlayerRegistry::setOnDisconnect(DisconnectHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    onDisconnect_ = std::move(handler);
}

void PlayerRegistry::setOnReconnect(ReconnectHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    onReconnect_ = std::move(handler);
}

std::vector<char> PlayerRegistry::extractExpiredDisconnects() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<char> expiredColors;
    auto now = std::chrono::steady_clock::now();
    std::vector<PendingDisconnect> stillPending;

    for (const PendingDisconnect& pending : pendingDisconnects_) {
        if (pending.expiresAt <= now) {
            expiredColors.push_back(pending.color);
            availableColors_.push_back(pending.color);
        } else {
            stillPending.push_back(pending);
        }
    }
    pendingDisconnects_ = stillPending;
    return expiredColors;
}
