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

#include "../../../lobby/lobby_registry.hpp"
#include "../../../lobby/matchmaker.hpp"
#include "../../../matches/game_registry.hpp"
#include "../../../network/websocket_server.hpp"
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

    bool receivedContains(std::mutex& mutex, std::vector<std::string>& received, const std::string& needle) {
        std::lock_guard<std::mutex> lock(mutex);
        for (const std::string& message : received) {
            if (message.find(needle) != std::string::npos) return true;
        }
        return false;
    }
}

TEST_CASE("two clients log in, get matched, and a click broadcasts to both over real WebSockets") {
    ix::initNetSystem();

    Board board(4, 4);
    board.setCell(0, 0, "wR");
    SqlitePlayerAccountStore accounts(":memory:");
    LobbyRegistry lobbyRegistry;
    GameRegistry gameRegistry;
    Matchmaker matchmaker(gameRegistry, accounts, board);

    const int port = 8901;
    GameWebSocketServer server(port, lobbyRegistry, gameRegistry, matchmaker, accounts);
    REQUIRE(server.start());

    std::atomic<bool> stopTicking{false};
    std::thread ticker([&]() {
        while (!stopTicking.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            matchmaker.tick();
            server.attachNewlyMatchedConnections();
            gameRegistry.updateAll(20);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::mutex mutexA, mutexB;
    std::vector<std::string> receivedA, receivedB;

    ix::WebSocket clientA;
    clientA.setUrl("ws://127.0.0.1:" + std::to_string(port));
    clientA.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::lock_guard<std::mutex> lock(mutexA);
            receivedA.push_back(msg->str);
        }
    });
    clientA.start();

    ix::WebSocket clientB;
    clientB.setUrl("ws://127.0.0.1:" + std::to_string(port));
    clientB.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::lock_guard<std::mutex> lock(mutexB);
            receivedB.push_back(msg->str);
        }
    });
    clientB.start();

    REQUIRE(waitFor([&]() { return clientA.getReadyState() == ix::ReadyState::Open; }));
    REQUIRE(waitFor([&]() { return clientB.getReadyState() == ix::ReadyState::Open; }));

    clientA.send(R"({"command":"login","username":"alice","password":"pw123"})");
    clientB.send(R"({"command":"login","username":"bob","password":"pw123"})");

    REQUIRE(waitFor([&]() { return receivedContains(mutexA, receivedA, "GameStartedEvent"); }));
    REQUIRE(waitFor([&]() { return receivedContains(mutexB, receivedB, "GameStartedEvent"); }));

    // Color assignment order isn't deterministic across the two clients, so both
    // send the same clicks — only the one actually holding white will take effect.
    clientA.send(R"({"command":"click","row":0,"col":0})");
    clientB.send(R"({"command":"click","row":0,"col":0})");
    clientA.send(R"({"command":"click","row":0,"col":3})");
    clientB.send(R"({"command":"click","row":0,"col":3})");

    REQUIRE(waitFor([&]() { return receivedContains(mutexA, receivedA, "MoveStartedEvent"); }));
    REQUIRE(waitFor([&]() { return receivedContains(mutexB, receivedB, "MoveStartedEvent"); }));

    clientA.stop();
    clientB.stop();
    stopTicking = true;
    ticker.join();
}

TEST_CASE("a client that reconnects with the same username is routed back into their existing match") {
    ix::initNetSystem();

    Board board(4, 4);
    board.setCell(0, 0, "wR");
    SqlitePlayerAccountStore accounts(":memory:");
    LobbyRegistry lobbyRegistry;
    GameRegistry gameRegistry;
    Matchmaker matchmaker(gameRegistry, accounts, board);

    const int port = 8903;
    GameWebSocketServer server(port, lobbyRegistry, gameRegistry, matchmaker, accounts);
    REQUIRE(server.start());

    std::atomic<bool> stopTicking{false};
    std::thread ticker([&]() {
        while (!stopTicking.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            matchmaker.tick();
            server.attachNewlyMatchedConnections();
            gameRegistry.updateAll(20);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::mutex mutexA, mutexB;
    std::vector<std::string> receivedA, receivedB;

    ix::WebSocket clientA;
    clientA.setUrl("ws://127.0.0.1:" + std::to_string(port));
    clientA.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::lock_guard<std::mutex> lock(mutexA);
            receivedA.push_back(msg->str);
        }
    });
    clientA.start();

    ix::WebSocket clientB;
    clientB.setUrl("ws://127.0.0.1:" + std::to_string(port));
    clientB.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::lock_guard<std::mutex> lock(mutexB);
            receivedB.push_back(msg->str);
        }
    });
    clientB.start();

    REQUIRE(waitFor([&]() { return clientA.getReadyState() == ix::ReadyState::Open; }));
    REQUIRE(waitFor([&]() { return clientB.getReadyState() == ix::ReadyState::Open; }));

    clientA.send(R"({"command":"login","username":"alice2","password":"pw123"})");
    clientB.send(R"({"command":"login","username":"bob2","password":"pw123"})");

    REQUIRE(waitFor([&]() { return receivedContains(mutexA, receivedA, "GameStartedEvent"); }));
    REQUIRE(waitFor([&]() { return receivedContains(mutexB, receivedB, "GameStartedEvent"); }));

    // alice2 disconnects and reconnects with the same username, on a fresh socket.
    clientA.stop();

    std::mutex mutexA2;
    std::vector<std::string> receivedA2;
    ix::WebSocket clientA2;
    clientA2.setUrl("ws://127.0.0.1:" + std::to_string(port));
    clientA2.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::lock_guard<std::mutex> lock(mutexA2);
            receivedA2.push_back(msg->str);
        }
    });
    clientA2.start();
    REQUIRE(waitFor([&]() { return clientA2.getReadyState() == ix::ReadyState::Open; }));

    clientA2.send(R"({"command":"login","username":"alice2","password":"pw123"})");

    // Being routed back into the same match sends a fresh GameStartedEvent snapshot.
    REQUIRE(waitFor([&]() { return receivedContains(mutexA2, receivedA2, "GameStartedEvent"); }));

    clientA2.stop();
    clientB.stop();
    stopTicking = true;
    ticker.join();
}

TEST_CASE("a client with no matching rating partner receives NoMatchFoundEvent after the matchmaking timeout") {
    ix::initNetSystem();

    Board board(4, 4);
    board.setCell(0, 0, "wR");
    SqlitePlayerAccountStore accounts(":memory:");
    LobbyRegistry lobbyRegistry;
    GameRegistry gameRegistry;
    Matchmaker matchmaker(gameRegistry, accounts, board, /*timeoutMs=*/50);

    const int port = 8904;
    GameWebSocketServer server(port, lobbyRegistry, gameRegistry, matchmaker, accounts);
    REQUIRE(server.start());

    std::atomic<bool> stopTicking{false};
    std::thread ticker([&]() {
        while (!stopTicking.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            matchmaker.tick();
            server.attachNewlyMatchedConnections();
            gameRegistry.updateAll(20);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::mutex mutex;
    std::vector<std::string> received;
    ix::WebSocket client;
    client.setUrl("ws://127.0.0.1:" + std::to_string(port));
    client.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::lock_guard<std::mutex> lock(mutex);
            received.push_back(msg->str);
        }
    });
    client.start();
    REQUIRE(waitFor([&]() { return client.getReadyState() == ix::ReadyState::Open; }));

    client.send(R"({"command":"login","username":"lonely","password":"pw123"})");

    REQUIRE(waitFor([&]() { return receivedContains(mutex, received, "NoMatchFoundEvent"); }));

    client.stop();
    stopTicking = true;
    ticker.join();
}
