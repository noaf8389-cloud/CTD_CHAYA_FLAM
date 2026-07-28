#pragma once

#include <any>
#include <cstddef>
#include <functional>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

class EventBus {
public:
    using SubscriptionId = std::size_t;

    template <typename EventT>
    SubscriptionId subscribe(std::function<void(const EventT&)> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto type = std::type_index(typeid(EventT));
        const SubscriptionId id = next_id_++;
        handlers_[type].push_back(Subscription{
            id,
            [handler](const std::any& event) {
                handler(std::any_cast<const EventT&>(event));
            }});
        return id;
    }

    template <typename EventT>
    void publish(const EventT& event) {
        std::vector<HandlerFn> handlers_snapshot;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            const auto type = std::type_index(typeid(EventT));
            const auto it = handlers_.find(type);
            if (it == handlers_.end()) {
                return;
            }
            for (const auto& subscription : it->second) {
                handlers_snapshot.push_back(subscription.handler);
            }
        }
        const std::any boxed_event = event;
        for (const auto& handler : handlers_snapshot) {
            handler(boxed_event);
        }
    }

    // Removes a previously registered subscription so its handler stops receiving events.
    void unsubscribe(SubscriptionId id);

private:
    using HandlerFn = std::function<void(const std::any&)>;

    struct Subscription {
        SubscriptionId id;
        HandlerFn handler;
    };

    std::mutex mutex_;
    std::unordered_map<std::type_index, std::vector<Subscription>> handlers_;
    SubscriptionId next_id_ = 1;
};
