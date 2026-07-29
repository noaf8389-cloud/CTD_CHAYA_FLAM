#include "test_framework.hpp"
#include "../src/game/network/network_receiver.hpp"
#include "../src/game/network/server_connection.hpp"
#include "bus/event_bus.hpp"

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

TEST(network_receiver_dispatches_login_result_event) {
    EventBus bus;
    ServerConnection connection("ws://localhost:1");
    NetworkReceiver receiver(connection, bus);

    std::vector<LoginResultReceived> results;
    bus.subscribe<LoginResultReceived>([&results](const LoginResultReceived& e) { results.push_back(e); });

    receiver.handleMessage(R"({"type":"LoginResultEvent","payload":{"success":true,"rating":1300}})");

    EXPECT_EQ(results.size(), size_t(1));
    EXPECT_TRUE(results[0].success);
    EXPECT_EQ(results[0].rating, 1300);
}

TEST(network_receiver_dispatches_room_created_event) {
    EventBus bus;
    ServerConnection connection("ws://localhost:1");
    NetworkReceiver receiver(connection, bus);

    std::vector<RoomCreatedReceived> created;
    bus.subscribe<RoomCreatedReceived>([&created](const RoomCreatedReceived& e) { created.push_back(e); });

    receiver.handleMessage(R"({"type":"RoomCreatedEvent","payload":{"roomCode":"ABC123"}})");

    EXPECT_EQ(created.size(), size_t(1));
    EXPECT_EQ(created[0].roomCode, std::string("ABC123"));
}

TEST(network_receiver_dispatches_join_room_failed_event) {
    EventBus bus;
    ServerConnection connection("ws://localhost:1");
    NetworkReceiver receiver(connection, bus);

    int callCount = 0;
    bus.subscribe<JoinRoomFailedReceived>([&callCount](const JoinRoomFailedReceived&) { callCount++; });

    receiver.handleMessage(R"({"type":"JoinRoomFailedEvent","payload":{}})");

    EXPECT_EQ(callCount, 1);
}

TEST(network_receiver_dispatches_no_match_found_event) {
    EventBus bus;
    ServerConnection connection("ws://localhost:1");
    NetworkReceiver receiver(connection, bus);

    int callCount = 0;
    bus.subscribe<NoMatchFoundReceived>([&callCount](const NoMatchFoundReceived&) { callCount++; });

    receiver.handleMessage(R"({"type":"NoMatchFoundEvent","payload":{}})");

    EXPECT_EQ(callCount, 1);
}
