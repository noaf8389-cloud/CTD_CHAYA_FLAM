#include "test_framework.hpp"
#include "../src/game/network/network_receiver.hpp"
#include "../src/game/network/server_connection.hpp"
#include "../src/game/bus/event_bus.hpp"

TEST(network_receiver_dispatches_move_made_event) {
    EventBus bus;
    ServerConnection connection("ws://localhost:1");  // לא מתחבר לשום מקום בפועל, לא נחוץ לטסט הזה
    NetworkReceiver receiver(connection, bus);

    std::vector<MoveMadeEvent> made;
    bus.subscribe<MoveMadeEvent>([&made](const MoveMadeEvent& e) { made.push_back(e); });

    receiver.handleMessage(R"({"type":"MoveMadeEvent","payload":{"from":{"row":0,"col":0},"to":{"row":0,"col":3},"piece_token":"wR","timestamp_ms":1000}})");

    EXPECT_EQ(made.size(), size_t(1));
    EXPECT_EQ(made[0].piece_token, std::string("wR"));
}

TEST(network_receiver_ignores_malformed_json) {
    EventBus bus;
    ServerConnection connection("ws://localhost:1");
    NetworkReceiver receiver(connection, bus);

    std::vector<MoveMadeEvent> made;
    bus.subscribe<MoveMadeEvent>([&made](const MoveMadeEvent& e) { made.push_back(e); });

    receiver.handleMessage("not valid json");

    EXPECT_TRUE(made.empty());
}

TEST(network_receiver_ignores_unknown_event_type) {
    EventBus bus;
    ServerConnection connection("ws://localhost:1");
    NetworkReceiver receiver(connection, bus);

    std::vector<MoveMadeEvent> made;
    bus.subscribe<MoveMadeEvent>([&made](const MoveMadeEvent& e) { made.push_back(e); });

    receiver.handleMessage(R"({"type":"SomethingUnknown","payload":{}})");

    EXPECT_TRUE(made.empty());
}

TEST(network_receiver_ignores_message_without_payload) {
    EventBus bus;
    ServerConnection connection("ws://localhost:1");
    NetworkReceiver receiver(connection, bus);

    std::vector<MoveMadeEvent> made;
    bus.subscribe<MoveMadeEvent>([&made](const MoveMadeEvent& e) { made.push_back(e); });

    receiver.handleMessage(R"({"type":"MoveMadeEvent"})");

    EXPECT_TRUE(made.empty());
}
