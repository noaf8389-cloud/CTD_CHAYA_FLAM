#include "server.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#include <ixwebsocket/IXNetSystem.h>

#include "lobby/lobby_registry.hpp"
#include "lobby/matchmaker.hpp"
#include "matches/game_registry.hpp"
#include "db/sqlite_player_account_store.hpp"
#include "logic/io/board_parser.hpp"
#include "network/websocket_server.hpp"
#include "logging/logger.hpp"

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

    Board templateBoard = loadStartingBoard(layoutPath_);
    SqlitePlayerAccountStore accounts("players.db");
    LobbyRegistry lobbyRegistry;
    GameRegistry gameRegistry;
    Matchmaker matchmaker(gameRegistry, accounts, templateBoard);

    GameWebSocketServer webSocketServer(port_, lobbyRegistry, gameRegistry, matchmaker, accounts, std::move(templateBoard));

    if (!webSocketServer.start()) {
        return 1;
    }

    std::cout << "Server listening on port " << port_ << std::endl;
    Logger::info("Server listening on port " + std::to_string(port_));

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(kTickMs));
        matchmaker.tick();
        webSocketServer.attachNewlyMatchedConnections();
        gameRegistry.updateAll(kTickMs);
    }
}
