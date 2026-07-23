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

namespace {
    bool waitFor(const std::function<bool()>& predicate, int timeoutMs = 2000) {
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

    const int port = 8901;
    GameWebSocketServer server(port, publisher, commandHandler);
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
