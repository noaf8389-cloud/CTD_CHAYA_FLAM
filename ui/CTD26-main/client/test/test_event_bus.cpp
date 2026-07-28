#include "test_framework.hpp"
#include "bus/event_bus.hpp"

namespace {
    struct TestEvent { int value; };
    struct OtherTestEvent { int value; };
}

TEST(event_bus_subscriber_receives_published_event_of_its_own_type) {
    EventBus bus;
    int received = -1;
    bus.subscribe<TestEvent>([&](const TestEvent& e) { received = e.value; });
    bus.publish(TestEvent{42});
    EXPECT_EQ(received, 42);
}

TEST(event_bus_subscriber_does_not_receive_events_of_a_different_type) {
    EventBus bus;
    bool received = false;
    bus.subscribe<TestEvent>([&](const TestEvent&) { received = true; });
    bus.publish(OtherTestEvent{7});
    EXPECT_TRUE(!received);
}

TEST(event_bus_publishing_with_no_subscribers_does_not_throw) {
    EventBus bus;
    EXPECT_NO_THROW(bus.publish(TestEvent{1}));
}

TEST(event_bus_multiple_subscribers_all_receive_the_event) {
    EventBus bus;
    int first = -1, second = -1;
    bus.subscribe<TestEvent>([&](const TestEvent& e) { first = e.value; });
    bus.subscribe<TestEvent>([&](const TestEvent& e) { second = e.value; });
    bus.publish(TestEvent{5});
    EXPECT_EQ(first, 5);
    EXPECT_EQ(second, 5);
}

TEST(event_bus_unsubscribe_stops_further_delivery) {
    EventBus bus;
    int callCount = 0;
    EventBus::SubscriptionId id = bus.subscribe<TestEvent>([&](const TestEvent&) { ++callCount; });
    bus.publish(TestEvent{1});
    bus.unsubscribe(id);
    bus.publish(TestEvent{2});
    EXPECT_EQ(callCount, 1);
}

TEST(event_bus_unsubscribing_one_id_does_not_affect_others) {
    EventBus bus;
    int firstCount = 0, secondCount = 0;
    EventBus::SubscriptionId firstId = bus.subscribe<TestEvent>([&](const TestEvent&) { ++firstCount; });
    bus.subscribe<TestEvent>([&](const TestEvent&) { ++secondCount; });
    bus.unsubscribe(firstId);
    bus.publish(TestEvent{1});
    EXPECT_EQ(firstCount, 0);
    EXPECT_EQ(secondCount, 1);
}

TEST(event_bus_unsubscribing_an_unknown_id_is_a_harmless_no_op) {
    EventBus bus;
    EXPECT_NO_THROW(bus.unsubscribe(9999));
}
