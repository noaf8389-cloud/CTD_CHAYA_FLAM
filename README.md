# KungFu Chess

A real-time chess variant ("KungFu Chess") — pieces move independently and simultaneously in real time rather than in turns, gated by per-piece movement/rest cooldowns instead of alternating turns.

## Architecture

The project is split into two independent programs that communicate only over a WebSocket connection (JSON messages), with no shared source files between them:

- **`server/`** — game logic + networking (`KungFuChessServer.exe`). Owns the board, rules, and real-time timing; publishes game events on a local event bus and broadcasts them as JSON to all connected clients.
- **`ui/CTD26-main/cpp/`** — OpenCV graphical client (`KungFuChess.exe`). Renders the board and piece animations, sends click/jump commands to the server, and reacts to server-broadcast events by maintaining its own local copy of the board state.

The server must be running before a client connects. Multiple clients can connect to the same running server and will all see the same board update live.

## Prerequisites

- CMake 3.16+
- A C++17 toolchain (Visual Studio 2022 on Windows)
- OpenCV_451: download from
  https://drive.google.com/drive/folders/14SeyjbNPvsgyLKM2omcVTlTX0wAQ-_Ox?usp=sharing
  and place it under `ui/CTD26-main/cpp/OpenCV_451`

## Building the server

cd server
cmake -B build
cmake --build build --config Debug



Produces `server/build/Debug/KungFuChessServer.exe`.

## Building the client

cd ui/CTD26-main/cpp
cmake -B build
cmake --build build --config Debug



Produces `ui/CTD26-main/cpp/build/Debug/KungFuChess.exe`.

## Running

1. Start the server first:

cd server/build/Debug
./KungFuChessServer.exe



Listens on port 8080 by default, using `ui/CTD26-main/layout_standard.txt` as the starting board. Pass a different layout file path as the first argument to override it.

2. Start a client (connects to `ws://127.0.0.1:8080` by default):

cd ui/CTD26-main/cpp/build/Debug
./KungFuChess.exe



Controls: left-click to select/move a piece, right-click to trigger a jump.

## Running tests

Server:
cd server/build
ctest -C Debug --output-on-failure



Client:
cd ui/CTD26-main/cpp/build
ctest -C Debug --output-on-failure



## Project structure

server/
logic/       game rules, board model, real-time arbiter
bus/         event bus + game event definitions + JSON serialization
network/     WebSocket server, command handling, event broadcasting
tests/

ui/CTD26-main/
cpp/
src/game/  rendering, input, animation, client-side event bus + network
test/
pieces1/, pieces3/   sprite asset themes
layout_standard.txt  default starting board



## Status

Implemented:
- Real-time server-authoritative game state, broadcast live to any number of connected clients
- Full event-driven flow: move/jump/rest/capture/game-over events published on the server and forwarded to clients
- Graceful disconnect handling at the network layer

Not yet implemented:
- Player identity / color assignment per connection — any connected client can currently move any piece
- Multiple concurrent game rooms — the server hosts a single game per process