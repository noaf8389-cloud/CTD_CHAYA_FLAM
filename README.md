# KungFu Chess

A real-time chess variant ("KungFu Chess") — pieces move independently and simultaneously in real time rather than in turns, gated by per-piece movement/rest cooldowns instead of alternating turns.

## Architecture

The project is split into two independent programs that communicate only over a WebSocket connection (JSON messages):

- **`server/`** — game logic + networking (`KungFuChessServer.exe`). Runs any number of concurrent matches: newly logged-in clients are queued and paired by rating (±100 ELO) into independent `GameMatch` instances, each with its own board, rules, real-time timing, and event bus, broadcasting JSON events only to that match's own clients.
- **`ui/CTD26-main/client/`** — OpenCV graphical client (`KungFuChess.exe`). Renders the board and piece animations, sends click/jump commands to the server, and reacts to server-broadcast events by maintaining its own local copy of the board state.
- **`shared/`** — code that is genuinely identical on both sides, compiled into both executables from this single location: the board/position data model, the event bus and event definitions/JSON serialization, and the file logger. Anything server- or client-specific (networking, rendering, rules) stays in its own tree, not here.

The server must be running before a client connects. Clients that log in around the same time and with a similar rating are matched into the same game; clients matched into different games don't see each other's boards. Reconnecting with the same username routes a client back into their still-active match.

## Prerequisites

- CMake 3.16+
- A C++17 toolchain (Visual Studio 2022 on Windows)
- OpenCV_451: download from
  https://drive.google.com/drive/folders/14SeyjbNPvsgyLKM2omcVTlTX0wAQ-_Ox?usp=sharing
  and place it under `ui/CTD26-main/client/OpenCV_451`

## Building the server

cd server
cmake -B build
cmake --build build --config Debug



Produces `server/build/Debug/KungFuChessServer.exe`.

## Building the client

cd ui/CTD26-main/client
cmake -B build
cmake --build build --config Debug



Produces `ui/CTD26-main/client/build/Debug/KungFuChess.exe`.

## Running

1. Start the server first:

cd server/build/Debug
./KungFuChessServer.exe



Listens on port 8080 by default, using `ui/CTD26-main/layout_standard.txt` as the starting board. Pass a different layout file path as the first argument to override it.

2. Start a client (connects to `ws://127.0.0.1:8080` by default):

cd ui/CTD26-main/client/build/Debug
./KungFuChess.exe



Controls: left-click to select/move a piece, right-click to trigger a jump.

## Running tests

Server:
cd server/build
ctest -C Debug --output-on-failure



Client:
cd ui/CTD26-main/client/build
ctest -C Debug --output-on-failure



## Project structure

shared/
model/       Board, Position — used directly by both server and client
bus/         event bus + game event definitions + JSON serialization
logging/     file logger (server.log / client.log)

server/
logic/       game rules, board model, real-time arbiter
network/     WebSocket server, command handling, login, event broadcasting
matches/     GameMatch (one running match) + GameRegistry (all active matches, by username)
lobby/       LobbyRegistry (connected/identified players) + Matchmaker (rating-based pairing)
db/          SQLite-backed player accounts (username/password/rating)
rating/      ELO calculation + rating updates on game over
tests/

ui/CTD26-main/
client/
src/game/  rendering, input, animation, client-side network
test/
pieces1/, pieces3/   sprite asset themes
layout_standard.txt  default starting board



## Status

Implemented:
- Real-time server-authoritative game state, broadcast live to a match's connected clients
- Full event-driven flow: move/jump/rest/capture/game-over events published on the server and forwarded to clients
- Per-connection color assignment (`PlayerRegistry`, scoped per match) — each client can only move its own color
- Username/password login with SQLite-backed accounts and ELO rating, updated automatically on game over
- Multiple concurrent matches with rating-based matchmaking (±100 ELO), including a no-match-found timeout
- Disconnect grace period (20s) with auto-forfeit if not resumed in time, and reconnecting into an in-progress match with the same username
- Castling
- File logging (`server.log` / `client.log`) for connections, logins, matchmaking, malformed messages, and errors

Not yet implemented:
- On-screen score, move log, and sound