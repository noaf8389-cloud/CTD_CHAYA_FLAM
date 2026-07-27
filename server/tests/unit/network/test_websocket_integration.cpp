#include "../../catch2/catch_amalgamated.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>

#include "../../../bus/event_bus.hpp"
#include "../../../game_session.hpp"
#include "../../../network/command_handler.hpp"
#include "../../../network/network_publisher.hpp"
#include "../../../network/websocket_server.hpp"
#include "../../../player_registry.hpp"
#include "../../../db/sqlite_player_account_store.hpp"

namespace {
    bool waitFor(const std::function<bool()>& predicate, int timeoutMs = 5000) {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
        while (std::chrono::steady_clock::now() < deadline) {
            if (predicate()) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return false;
    }
}

TEST_CASE("a real client receives a MoveStartedEvent broadcast after sending click commands over WebSocket") {
    ix::initNetSystem();

    Board board(4, 4);
    board.setCell(0, 0, "wR");
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);
    NetworkPublisher publisher(bus, session);
    CommandHandler commandHandler(session);
    PlayerRegistry playerRegistry;
    SqlitePlayerAccountStore accounts(":memory:");

    const int port = 8901;
    GameWebSocketServer server(port, publisher, commandHandler, playerRegistry, accounts);
    REQUIRE(server.start());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::mutex messagesMutex;
    std::vector<std::string> receivedMessages;

    ix::WebSocket client;
    client.setUrl("ws://127.0.0.1:" + std::to_string(port));
    client.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::lock_guard<std::mutex> lock(messagesMutex);
            receivedMessages.push_back(msg->str);
        }
    });
    client.start();

    REQUIRE(waitFor([&]() { return client.getReadyState() == ix::ReadyState::Open; }));

    client.send(R"({"command":"click","row":0,"col":0})");
    client.send(R"({"command":"click","row":0,"col":3})");

    auto containsMoveStarted = [&]() {
        std::lock_guard<std::mutex> lock(messagesMutex);
        for (const std::string& message : receivedMessages) {
            if (message.find("MoveStartedEvent") != std::string::npos) return true;
        }
        return false;
    };

    REQUIRE(waitFor(containsMoveStarted));

    client.stop();
}

TEST_CASE("a real client receives move, jump and rest lifecycle events over WebSocket", "[lifecycle]") {
    ix::initNetSystem();

    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(3, 3, "wN");
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);
    NetworkPublisher publisher(bus, session);
    CommandHandler commandHandler(session);
    PlayerRegistry playerRegistry;
    SqlitePlayerAccountStore accounts(":memory:");

    const int port = 8902;
    GameWebSocketServer server(port, publisher, commandHandler, playerRegistry, accounts);
    REQUIRE(server.start());

    std::atomic<bool> stopTicking{false};
    std::thread ticker([&]() {
        while (!stopTicking.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            session.update(20);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::mutex messagesMutex;
    std::vector<std::string> receivedMessages;

    ix::WebSocket client;
    client.setUrl("ws://127.0.0.1:" + std::to_string(port));
    client.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::lock_guard<std::mutex> lock(messagesMutex);
            receivedMessages.push_back(msg->str);
        }
    });
    client.start();

    REQUIRE(waitFor([&]() { return client.getReadyState() == ix::ReadyState::Open; }));

    client.send(R"({"command":"click","row":0,"col":0})");
    client.send(R"({"command":"click","row":0,"col":3})");
    client.send(R"({"command":"jump","row":3,"col":3})");

    auto receivedContaining = [&](const std::string& needle) {
        std::lock_guard<std::mutex> lock(messagesMutex);
        for (const std::string& message : receivedMessages) {
            if (message.find(needle) != std::string::npos) return true;
        }
        return false;
    };

    REQUIRE(waitFor([&]() { return receivedContaining("MoveMadeEvent"); }));
    REQUIRE(waitFor([&]() { return receivedContaining("JumpStartedEvent"); }));
    REQUIRE(waitFor([&]() { return receivedContaining("JumpLandedEvent"); }));
    REQUIRE(waitFor([&]() { return receivedContaining("RestEndedEvent"); }));

    client.stop();
    stopTicking = true;
    ticker.join();
}
