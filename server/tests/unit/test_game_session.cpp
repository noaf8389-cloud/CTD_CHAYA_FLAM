#include "../catch2/catch_amalgamated.hpp"
#include "../../game_session.hpp"
#include "../../bus/game_events.hpp"

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

    session.handleClick(0, 0);    // select (0,0)
    session.handleClick(0, 3);    // move to (0,3)

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

    session.handleClick(0, 0);

    REQUIRE(started.empty());
}

TEST_CASE("GameSession::handleClick publishes nothing when the target is illegal") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<MoveStartedEvent> started;
    bus.subscribe<MoveStartedEvent>([&started](const MoveStartedEvent& e) { started.push_back(e); });

    session.handleClick(0, 0);
    session.handleClick(1, 1);  // אלכסון - לא חוקי לצריח

    REQUIRE(started.empty());
}

TEST_CASE("GameSession::update publishes MoveMadeEvent once a move completes") {
    GameState gameState(makeBoardWithRook());
    EventBus bus;
    GameSession session(gameState, bus);

    std::vector<MoveMadeEvent> made;
    bus.subscribe<MoveMadeEvent>([&made](const MoveMadeEvent& e) { made.push_back(e); });

    session.handleClick(0, 0);
    session.handleClick(0, 3);
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

    session.handleClick(0, 0);
    session.handleClick(0, 3);
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

    session.handleClick(0, 0);
    session.handleClick(0, 3);
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

    session.handleClick(0, 0);
    session.handleClick(0, 3);
    session.update(100000);   // המהלך מסתיים, ה-rest הארוך מתחיל

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

    session.handleJump(1, 1);
    session.update(GameState::JUMP_DURATION_MS + 1);   // הקפיצה נוחתת, ה-rest הקצר מתחיל

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

    session.handleClick(0, 0);
    session.handleClick(0, 3);
    session.update(100000);

    std::vector<RestEndedEvent> restEnded;
    bus.subscribe<RestEndedEvent>([&restEnded](const RestEndedEvent& e) { restEnded.push_back(e); });

    session.update(1);   // עדיין בתוך תקופת המנוחה

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

    session.handleJump(1, 1);

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

    session.handleJump(1, 1);

    REQUIRE(started.empty());
}

TEST_CASE("GameSession::update publishes JumpLandedEvent once the jump's airborne time elapses") {
    Board board(3, 3);
    board.setCell(1, 1, "wK");
    GameState gameState(board);
    EventBus bus;
    GameSession session(gameState, bus);

    session.handleJump(1, 1);

    std::vector<JumpLandedEvent> landed;
    bus.subscribe<JumpLandedEvent>([&landed](const JumpLandedEvent& e) { landed.push_back(e); });

    session.update(GameState::JUMP_DURATION_MS + 1);

    REQUIRE(landed.size() == 1);
    REQUIRE(landed[0].position == Position{1, 1});
    REQUIRE(landed[0].piece_token == "wK");
}
