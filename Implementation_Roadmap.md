# Cloud Migration — Implementation Roadmap

This is a **living document**. It gets updated as each step is actually executed — with the decision made, the alternative considered, and any deviation from the plan. It is the companion to [`Server_Design.md`](Server_Design.md) (the target architecture) — this file is the *path* to get there from the current single-process codebase.

## Guiding principles

- **Prepare before building.** Cut the internal boundaries between the future services (API Gateway / WS Gateway / Matchmaker / Game Allocator / Game Server Shard) inside the current single-process server *before* touching Redis/NATS/PostgreSQL/Kubernetes at all.
- **No rewrites.** Each file, once written for a given step, should survive later steps unchanged in shape — later steps add (e.g. a new implementation of an existing interface), they don't restructure.
- **Encapsulation is preserved throughout.** Every new class knows only the interface below it, never the technology behind that interface (the existing `PlayerAccountStore` abstraction over SQLite is the model to repeat).
- **Ordered, not all-at-once.** Each step below is independently committable and testable. Small steps are fine; skipping ahead is not.
- **Alternatives are evaluated at execution time**, not decided in advance — each step has a "Decision Log" subsection filled in when that step is actually done.
- **Speed and smooth scaling matter more than cost.** When a step involves a speed-vs-precision tradeoff, that tradeoff is called out explicitly in its Decision Log.

## Status legend

`Not started` · `In progress` · `Done` · `Skipped (reason noted)`

---

## Phase 0 — Prep: cut the boundaries in the existing code

No protocol or network changes yet. Same single executable, same WebSocket, same external behavior (`test_websocket_integration.cpp` keeps passing unchanged) — the code is just reorganized to already match the future service boundaries.

### 0.1 — Extract `AuthService`
**Status:** Not started
**Goal:** Pull the business logic (not the networking) out of `GameWebSocketServer::handleLogin` into a new class that depends only on `PlayerAccountStore&` + `LobbyRegistry&` — no knowledge of sockets.
**Files:** `server/auth/auth_service.hpp/.cpp` (new), `server/network/websocket_server.cpp` (shrinks to call `AuthService`)
**Why here:** Seed of the future API Gateway. `handleLogin` today is already nearly "pure" business logic aside from the final `state.socket->send(...)` — low-risk extraction.
**Decision Log:** _(filled in when executed)_

### 0.2 — Extract `RoomsService`
**Status:** Not started
**Goal:** Pull `handleCreateRoom`/`handleJoinRoom` logic into a class depending only on `GameRegistry&` + `PlayerAccountStore&`.
**Files:** `server/rooms/rooms_service.hpp/.cpp` (new)
**Why here:** Second seed of the future API Gateway.
**Decision Log:** _(filled in when executed)_

### 0.3 — Introduce a thin `GameAllocator`
**Status:** Not started
**Goal:** Add `GameAllocator::allocate() -> GameRegistry&`. Today it trivially returns the single existing in-process registry — no real allocation logic yet.
**Files:** `server/matches/game_allocator.hpp/.cpp` (new)
**Why here:** The most important seam to open early. When multiple shards exist later, only this function's internals change — no call site does.
**Decision Log:** _(filled in when executed)_

### 0.4 — Shrink `GameWebSocketServer` to pure routing
**Status:** Not started
**Goal:** `GameWebSocketServer` becomes routing-only: parse the raw message, identify its kind, delegate to `AuthService` / `RoomsService` / `Matchmaker` / `GameAllocator`. It no longer knows *how* login or room-joining work.
**Files:** `server/network/websocket_server.cpp/.hpp`
**Why here:** This makes it, in practice, the seed of the future WS Gateway — routing is its only remaining job.
**Decision Log:** _(filled in when executed)_

### 0.5 — Unit tests for the new services
**Status:** Not started
**Goal:** `AuthService`/`RoomsService`/`GameAllocator` get their own unit tests (no real socket involved) — additive to, not a replacement for, `test_websocket_integration.cpp`.
**Files:** `server/tests/unit/auth/...`, `server/tests/unit/rooms/...`
**Why here:** Confirms the extraction didn't change behavior, and gives faster feedback than the full socket integration suite.
**Decision Log:** _(filled in when executed)_

**End-of-phase check:** same executable, identical external behavior, existing integration tests pass unchanged.

---

## Phase 1 — Split transport: real REST alongside WebSocket

**Status:** Not started
**Goal:** Add a real HTTP listener (alongside the existing WebSocket, not replacing it) that exposes `AuthService`/`RoomsService`. This is the point where client↔API-Gateway and client↔WS-Gateway become two distinct network channels, not just two internal classes.
**Decision to make here:** ixwebsocket's built-in basic HTTP server vs. a dedicated REST library (e.g. cpp-httplib) — implementation speed vs. feature completeness.
**Decision Log:** _(filled in when executed)_

---

## Phase 2 — Durable storage: PostgreSQL

**Status:** Not started
**Goal:** Implement `PostgresPlayerAccountStore`, matching the existing `PlayerAccountStore` interface exactly.
**Why here:** Lowest risk in the whole plan — the interface already exists and is proven (`SqlitePlayerAccountStore`). No call site changes.
**Decision Log:** _(filled in when executed)_

---

## Phase 3 — Shared ephemeral state: Redis

**Status:** Not started
**Goal:** Replace the in-process maps in `GameRegistry`/`LobbyRegistry`/`Matchmaker` (`usernameToGame_`, `roomCodeToGame_`, the waiting queue) with Redis-backed implementations — behind the exact same public interfaces (`matchFor`, `createRoom`, `enqueue`, etc. keep their signatures).
**Why here:** Only safe because Phase 0 already made these clean classes with clear interfaces — this becomes a technical swap, not a redesign.
**Decision to make here:** hiredis (raw) vs. redis-plus-plus (higher-level API).
**Decision Log:** _(filled in when executed)_

---

## Phase 4 — Inter-service channel: NATS

**Status:** Not started
**Goal:** Publish to NATS alongside the existing in-process `EventBus` (not replacing it yet) — proves the inter-service protocol works before anything depends on it.
**Decision to make here:** NATS core (plain pub/sub) vs. JetStream (persistence) — see `Server_Design.md` §4.
**Decision Log:** _(filled in when executed)_

---

## Phase 5 — Actual process/container split

**Status:** Not started
**Goal:** `AuthService` + `RoomsService` (from Phase 0–1) move into a standalone executable that becomes the real **API Gateway**. The routing-only `GameWebSocketServer` (from 0.4) becomes the real **WS Gateway**. `Matchmaker` moves to its own process. Because Redis (Phase 3) and NATS (Phase 4) already work, this is mostly "move files into a new project and write a new `main()`" — not a redesign.
**Decision Log:** _(filled in when executed)_

---

## Phase 6 — Real Game Allocator + multiple shards

**Status:** Not started
**Goal:** Implement real allocation logic (load/region-based) inside the seam opened in 0.3 — only meaningful now that more than one shard exists to choose between.
**Decision Log:** _(filled in when executed)_

---

## Phase 7 — Observability

**Status:** Not started
**Goal:** Centralized logs, metrics, health checks — before Kubernetes, so Phase 8 is debuggable at all.
**Decision Log:** _(filled in when executed)_

---

## Phase 8 — Docker Compose → Kubernetes/K3s

**Status:** Not started
**Goal:** Local Docker Compose environment first, then production Kubernetes/K3s deployment with autoscaling, optionally via Agones for the shards.
**Decision Log:** _(filled in when executed)_

---

## Change history

- **2026-07-29** — Roadmap created, translated from the Hebrew planning discussion. No implementation started yet.
