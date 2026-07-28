#include "../catch2/catch_amalgamated.hpp"
#include "../../game_session.hpp"
#include "bus/game_events.hpp"

namespace {
    Board makeBoardWithRook() {
        Board board(4, 4);
        board.setCell(0, 0, "wR");
        return board;
    }
}

TEST_CASE("GameSession::handleClick publishes MoveStartedEvent for a legal move") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<MoveStartedEvent> started;
    bus.subscribe<MoveStartedEvent>([&started](const MoveStartedEvent& e) { started.push_back(e); });

    session.handleClick(0, 0, 'w');
    session.handleClick(0, 3, 'w');

    REQUIRE(started.size() == 1);
    REQUIRE(started[0].from.row == 0);
    REQUIRE(started[0].from.col == 0);
    REQUIRE(started[0].to.row == 0);
    REQUIRE(started[0].to.col == 3);
    REQUIRE(started[0].piece_token == "wR");
    REQUIRE(started[0].duration_ms == 3 * GameState::MS_PER_CELL);
}

TEST_CASE("GameSession::handleClick publishes nothing when the click only selects") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<MoveStartedEvent> started;
    bus.subscribe<MoveStartedEvent>([&started](const MoveStartedEvent& e) { started.push_back(e); });

    session.handleClick(0, 0, 'w');

    REQUIRE(started.empty());
}

TEST_CASE("GameSession::handleClick publishes nothing when the target is illegal") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<MoveStartedEvent> started;
    bus.subscribe<MoveStartedEvent>([&started](const MoveStartedEvent& e) { started.push_back(e); });

    session.handleClick(0, 0, 'w');
    session.handleClick(1, 1, 'w');

    REQUIRE(started.empty());
}

TEST_CASE("GameSession::handleClick ignores a click from the wrong acting color") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<MoveStartedEvent> started;
    bus.subscribe<MoveStartedEvent>([&started](const MoveStartedEvent& e) { started.push_back(e); });

    session.handleClick(0, 0, 'b');   // wR at (0,0), but acting as black
    session.handleClick(0, 3, 'b');

    REQUIRE(started.empty());
}

TEST_CASE("GameSession::update publishes MoveMadeEvent once a move completes") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<MoveMadeEvent> made;
    bus.subscribe<MoveMadeEvent>([&made](const MoveMadeEvent& e) { made.push_back(e); });

    session.handleClick(0, 0, 'w');
    session.handleClick(0, 3, 'w');
    session.update(100000);

    REQUIRE(made.size() == 1);
    REQUIRE(made[0].to.row == 0);
    REQUIRE(made[0].to.col == 3);
    REQUIRE(made[0].piece_token == "wR");
}

TEST_CASE("GameSession::update publishes PieceCapturedEvent on a capture") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 3, "bK");
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<PieceCapturedEvent> captured;
    bus.subscribe<PieceCapturedEvent>([&captured](const PieceCapturedEvent& e) { captured.push_back(e); });

    session.handleClick(0, 0, 'w');
    session.handleClick(0, 3, 'w');
    session.update(100000);

    REQUIRE(captured.size() == 1);
    REQUIRE(captured[0].capturing_piece_token == "wR");
    REQUIRE(captured[0].captured_piece_token == "bK");
}

TEST_CASE("GameSession::update publishes GameOverEvent when a king is captured") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 3, "bK");
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<GameOverEvent> over;
    bus.subscribe<GameOverEvent>([&over](const GameOverEvent& e) { over.push_back(e); });

    session.handleClick(0, 0, 'w');
    session.handleClick(0, 3, 'w');
    session.update(100000);

    REQUIRE(over.size() == 1);
    REQUIRE(over[0].winner_color == 'w');
}

TEST_CASE("GameSession::update publishes nothing when there is no pending move") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<MoveMadeEvent> made;
    bus.subscribe<MoveMadeEvent>([&made](const MoveMadeEvent& e) { made.push_back(e); });

    session.update(1000);

    REQUIRE(made.empty());
}

TEST_CASE("GameSession::update publishes RestEndedEvent once a post-move long rest elapses") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    session.handleClick(0, 0, 'w');
    session.handleClick(0, 3, 'w');
    session.update(100000);

    std::vector<RestEndedEvent> restEnded;
    bus.subscribe<RestEndedEvent>([&restEnded](const RestEndedEvent& e) { restEnded.push_back(e); });

    session.update(GameState::LONG_REST_DURATION_MS + 1);

    REQUIRE(restEnded.size() == 1);
    REQUIRE(restEnded[0].position == Position{0, 3});
    REQUIRE(restEnded[0].piece_token == "wR");
}

TEST_CASE("GameSession::update publishes RestEndedEvent once a post-jump short rest elapses") {
    Board board(3, 3);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);

    session.handleJump(1, 1, 'w');
    session.update(GameState::JUMP_DURATION_MS + 1);

    std::vector<RestEndedEvent> restEnded;
    bus.subscribe<RestEndedEvent>([&restEnded](const RestEndedEvent& e) { restEnded.push_back(e); });

    session.update(GameState::SHORT_REST_DURATION_MS + 1);

    REQUIRE(restEnded.size() == 1);
    REQUIRE(restEnded[0].position == Position{1, 1});
    REQUIRE(restEnded[0].piece_token == "wK");
}

TEST_CASE("GameSession::update publishes nothing when no rest has expired yet") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    session.handleClick(0, 0, 'w');
    session.handleClick(0, 3, 'w');
    session.update(100000);

    std::vector<RestEndedEvent> restEnded;
    bus.subscribe<RestEndedEvent>([&restEnded](const RestEndedEvent& e) { restEnded.push_back(e); });

    session.update(1);

    REQUIRE(restEnded.empty());
}

TEST_CASE("GameSession::handleJump publishes JumpStartedEvent for a stationary piece") {
    Board board(3, 3);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<JumpStartedEvent> started;
    bus.subscribe<JumpStartedEvent>([&started](const JumpStartedEvent& e) { started.push_back(e); });

    session.handleJump(1, 1, 'w');

    REQUIRE(started.size() == 1);
    REQUIRE(started[0].position == Position{1, 1});
    REQUIRE(started[0].piece_token == "wK");
    REQUIRE(started[0].duration_ms == GameState::JUMP_DURATION_MS);
}

TEST_CASE("GameSession::handleJump publishes nothing on an empty cell") {
    Board board(3, 3);
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<JumpStartedEvent> started;
    bus.subscribe<JumpStartedEvent>([&started](const JumpStartedEvent& e) { started.push_back(e); });

    session.handleJump(1, 1, 'w');

    REQUIRE(started.empty());
}

TEST_CASE("GameSession::handleJump publishes nothing when the piece belongs to the wrong color") {
    Board board(3, 3);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<JumpStartedEvent> started;
    bus.subscribe<JumpStartedEvent>([&started](const JumpStartedEvent& e) { started.push_back(e); });

    session.handleJump(1, 1, 'b');

    REQUIRE(started.empty());
}

TEST_CASE("GameSession::forfeit publishes GameOverEvent naming the opponent as winner") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<GameOverEvent> over;
    bus.subscribe<GameOverEvent>([&over](const GameOverEvent& e) { over.push_back(e); });

    session.forfeit('w');

    REQUIRE(over.size() == 1);
    REQUIRE(over[0].winner_color == 'b');
}

TEST_CASE("GameSession::forfeit ends the game") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    session.forfeit('w');

    REQUIRE(gameState.isGameOver());
}

TEST_CASE("GameSession::forfeit does nothing once the game is already over") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);
    session.forfeit('w');

    std::vector<GameOverEvent> over;
    bus.subscribe<GameOverEvent>([&over](const GameOverEvent& e) { over.push_back(e); });

    session.forfeit('b');

    REQUIRE(over.empty());
}

TEST_CASE("GameSession::playerDisconnected publishes PlayerDisconnectedEvent") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<PlayerDisconnectedEvent> disconnected;
    bus.subscribe<PlayerDisconnectedEvent>([&disconnected](const PlayerDisconnectedEvent& e) { disconnected.push_back(e); });

    session.playerDisconnected('b', 20000);

    REQUIRE(disconnected.size() == 1);
    REQUIRE(disconnected[0].color == 'b');
    REQUIRE(disconnected[0].grace_duration_ms == 20000);
}

TEST_CASE("GameSession::playerReconnected publishes PlayerReconnectedEvent") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<PlayerReconnectedEvent> reconnected;
    bus.subscribe<PlayerReconnectedEvent>([&reconnected](const PlayerReconnectedEvent& e) { reconnected.push_back(e); });

    session.playerReconnected('b');

    REQUIRE(reconnected.size() == 1);
    REQUIRE(reconnected[0].color == 'b');
}

TEST_CASE("GameSession::update publishes JumpLandedEvent once the jump's airborne time elapses") {
    Board board(3, 3);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);

    session.handleJump(1, 1, 'w');

    std::vector<JumpLandedEvent> landed;
    bus.subscribe<JumpLandedEvent>([&landed](const JumpLandedEvent& e) { landed.push_back(e); });

    session.update(GameState::JUMP_DURATION_MS + 1);

    REQUIRE(landed.size() == 1);
    REQUIRE(landed[0].position == Position{1, 1});
    REQUIRE(landed[0].piece_token == "wK");
}
