#include "server.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#include <ixwebsocket/IXNetSystem.h>

#include "bus/event_bus.hpp"
#include "game_session.hpp"
#include "logic/io/board_parser.hpp"
#include "network/command_handler.hpp"
#include "network/network_publisher.hpp"
#include "network/websocket_server.hpp"

namespace {
    constexpr long long kTickMs = 50;

    Board loadStartingBoard(const std::string& layoutPath) {
        std::ifstream file(layoutPath);
        Board board(0, 0);
        std::vector<std::string> commands;
        BoardParser::parse(file, board, commands);
        return board;
    }
}

int Server::run() {
    ix::initNetSystem();

    Board board = loadStartingBoard(layoutPath_);
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);

    NetworkPublisher publisher(bus, session);
    CommandHandler commandHandler(session);
    GameWebSocketServer webSocketServer(port_, publisher, commandHandler);

    if (!webSocketServer.start()) {
        return 1;
    }

    std::cout << "Server listening on port " << port_ << std::endl;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(kTickMs));
        session.update(kTickMs);
    }
}
