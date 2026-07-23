#include "command_handler.hpp"

#include "../game_session.hpp"
#include "../third_party/nlohmann/json.hpp"

using json = nlohmann::json;

CommandHandler::CommandHandler(GameSession& session){
    handlers_["click"] = [&session](int row, int col) { session.handleClick(row, col); };
    handlers_["jump"] = [&session](int row, int col) { session.handleJump(row, col); };
}

void CommandHandler::handleMessage(const std::string& rawMessage) {
    json parsed;
    try {
        parsed = json::parse(rawMessage);
    } catch (const json::parse_error&) {
        return;
    }

    const std::string command = parsed.value("command", "");
    const auto it = handlers_.find(command);
    if (it == handlers_.end()) {
        return;
    }

    const int row = parsed.value("row", 0);
    const int col = parsed.value("col", 0);
    it->second(row, col);
}
