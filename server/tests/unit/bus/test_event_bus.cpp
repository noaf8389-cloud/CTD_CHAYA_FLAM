#include "../../catch2/catch_amalgamated.hpp"

#include "bus/event_bus.hpp"

namespace {
    struct TestEvent {
        int value;
    };

    struct OtherTestEvent {
        int value;
    };
}

TEST_CASE("a subscriber receives a published event of its own type") {
    EventBus bus;
    int received = -1;
    bus.subscribe<TestEvent>([&](const TestEvent& e) { received = e.value; });

    bus.publish(TestEvent{42});

    REQUIRE(received == 42);
}

TEST_CASE("a subscriber does not receive events of a different type") {
    EventBus bus;
    bool received = false;
    bus.subscribe<TestEvent>([&](const TestEvent&) { received = true; });

    bus.publish(OtherTestEvent{7});

    REQUIRE_FALSE(received);
}

TEST_CASE("publishing an event with no subscribers does not throw") {
    EventBus bus;
    REQUIRE_NOTHROW(bus.publish(TestEvent{1}));
}

TEST_CASE("multiple subscribers to the same event type all receive it") {
    EventBus bus;
    int firstReceived = -1;
    int secondReceived = -1;
    bus.subscribe<TestEvent>([&](const TestEvent& e) { firstReceived = e.value; });
    bus.subscribe<TestEvent>([&](const TestEvent& e) { secondReceived = e.value; });

    bus.publish(TestEvent{5});

    REQUIRE(firstReceived == 5);
    REQUIRE(secondReceived == 5);
}

TEST_CASE("unsubscribe stops a handler from receiving further events") {
    EventBus bus;
    int callCount = 0;
    EventBus::SubscriptionId id = bus.subscribe<TestEvent>([&](const TestEvent&) { ++callCount; });

    bus.publish(TestEvent{1});
    bus.unsubscribe(id);
    bus.publish(TestEvent{2});

    REQUIRE(callCount == 1);
}

TEST_CASE("unsubscribing one subscription does not affect other subscriptions to the same event type") {
    EventBus bus;
    int firstCallCount = 0;
    int secondCallCount = 0;
    EventBus::SubscriptionId firstId = bus.subscribe<TestEvent>([&](const TestEvent&) { ++firstCallCount; });
    bus.subscribe<TestEvent>([&](const TestEvent&) { ++secondCallCount; });

    bus.unsubscribe(firstId);
    bus.publish(TestEvent{1});

    REQUIRE(firstCallCount == 0);
    REQUIRE(secondCallCount == 1);
}

TEST_CASE("unsubscribing an unknown id is a harmless no-op") {
    EventBus bus;
    REQUIRE_NOTHROW(bus.unsubscribe(9999));
}
