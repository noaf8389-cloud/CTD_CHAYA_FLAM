#include "event_bus.hpp"

#include <algorithm>

void EventBus::unsubscribe(SubscriptionId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [type, subscriptions] : handlers_) {
        subscriptions.erase(
            std::remove_if(subscriptions.begin(), subscriptions.end(),
                            [id](const Subscription& s) { return s.id == id; }),
            subscriptions.end());
    }
}
