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
**Status:** Done
**Goal:** Pull the business logic (not the networking) out of `GameWebSocketServer::handleLogin` into a new class that depends only on `PlayerAccountStore&` + `LobbyRegistry&` — no knowledge of sockets.
**Files:** `server/auth/auth_service.hpp/.cpp` (new), `server/network/websocket_server.cpp` (shrinks to call `AuthService`)
**Why here:** Seed of the future API Gateway. `handleLogin` today is already nearly "pure" business logic aside from the final `state.socket->send(...)` — low-risk extraction.
**Decision Log:** `AuthService::login(lobbyId, username, password) -> AuthResult{success, rating, existingMatch}` — takes `GameRegistry&` too (not just accounts/lobby) so it can resolve a resumable match on successful login, matching what `handleLogin` used to do inline. `GameWebSocketServer::attachToMatch` and the actual `socket->send(...)` stayed in `websocket_server.cpp` — those are genuinely WS-Gateway concerns (the live connection), not auth logic. `server/CMakeLists.txt` needed a new `AUTH_SOURCES` glob (new top-level folder, not auto-collected). Unit tests added in `server/tests/unit/auth/test_auth_service.cpp` using a real `SqlitePlayerAccountStore(":memory:")` (same pattern as the integration tests) rather than a hand-rolled fake, so the wrong-password path is exercised for real.

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
**Status:** In progress — `AuthService` covered (done alongside 0.1); `RoomsService`/`GameAllocator` still pending their own steps (0.2/0.3)
**Goal:** `AuthService`/`RoomsService`/`GameAllocator` get their own unit tests (no real socket involved) — additive to, not a replacement for, `test_websocket_integration.cpp`.
**Files:** `server/tests/unit/auth/test_auth_service.cpp` (done), `server/tests/unit/rooms/...` (pending)
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

**Status:** Done
**Goal:** Implement `PostgresPlayerAccountStore`, matching the existing `PlayerAccountStore` interface exactly.
**Why here:** Lowest risk in the whole plan — the interface already exists and is proven (`SqlitePlayerAccountStore`). No call site changes.
**Decision Log:**
- `libpqxx` couldn't be installed natively on Windows via vcpkg — every MSYS2 mirror download failed with HTTP 418 from local network interference (antivirus/firewall). Pivoted to building/testing `PostgresPlayerAccountStore` inside a Linux Docker container instead (`apt-get install libpqxx-dev`), which sidesteps the blocker entirely rather than working around it.
- Getting Docker Desktop itself running on this machine (ASUS Vivobook, Windows) required a full round of virtualization troubleshooting: BIOS-level VT-x was already on, but `VirtualMachinePlatform`/`Microsoft-Windows-Subsystem-Linux`/`HypervisorPlatform` Windows features needed explicit enabling via `dism.exe`, plus `wsl --update` / `wsl --set-default-version 2`, plus a genuine full restart each time (not just closing/reopening a terminal). `wsl --install -d Ubuntu` succeeding was the actual confirmation the fix took.
- `server/Dockerfile` (new) and `docker-compose.yml` (new, repo root) run a `tests` service against a real `postgres:16` container — this is the project's first working Docker artifact, a small preview of Phase 8, done now only because it was the fastest way to unblock this phase.
- Git-in-container `FetchContent` for `ixwebsocket` failed with `server certificate verification failed` — same class of local network interference as the vcpkg issue. Worked around with `git config --global http.sslVerify false` inside the Dockerfile; flagged as a temporary tradeoff (build-time only, not runtime) pending a real root-cause fix (finding what's doing TLS interception and trusting its CA properly instead).
- Found and fixed a real portability bug unrelated to Postgres: `server/DB` (disk, uppercase) vs. `db` (every `#include`/CMake reference, lowercase) — silently fine on Windows' case-insensitive filesystem, fatal on Linux. Renamed via `git mv` (two-step, since plain `Rename-Item` refuses case-only renames).
- Found and fixed a second portability bug: `shared/logging/logger.cpp` used `localtime_s` (MSVC-only); added `#ifdef _WIN32` / `localtime_r` fallback for Linux.
- `PostgresPlayerAccountStore.hpp` originally forward-declared `namespace pqxx { class connection; }`, assuming modern libpqxx 7.x. Ubuntu 22.04's packaged `libpqxx-dev` is 6.4.5, where `connection` is a type alias, not a class — the forward declaration conflicted with the real header and cascaded into dozens of unrelated-looking parse errors. Fixed by `#include <pqxx/pqxx>` directly in the header instead of forward-declaring.
- `CMakeLists.txt`: added `pkg_check_modules(PQXX libpqxx)` (optional, not `REQUIRED`) so the Postgres store and its test file are silently excluded from the native Windows build (no `libpqxx` there) but included automatically inside the Linux container (found via `apt`'s pkg-config files). Needed the `PQXX_FOUND` include/link block added to **both** `KungFuChessServer` and `KungFuChessServerTests` — missing it on the second one caused a linker error (headers resolved via `/usr/include`'s default path, masking that the library itself was never linked).
- `docker-compose.yml`'s plain `depends_on: - postgres` only waits for the container to start, not for Postgres to finish `initdb` and accept connections — caused one flaky test failure (`Connection refused`) on the first successful build. Fixed with a proper `healthcheck` (`pg_isready`) and `depends_on: postgres: condition: service_healthy`.
- End state: `docker compose up --build` runs the full existing test suite (433 tests, including new `PostgresPlayerAccountStore` and `AuthService` tests) against a real containerized PostgreSQL — all passing.

---

## Phase 3 — Shared ephemeral state: Redis

**Status:** Done — `LobbyRegistry` storage extracted behind `LobbyStore`; both `InMemoryLobbyStore` (default, unchanged behavior) and `RedisLobbyStore` (hiredis-backed) implemented and verified: native `ctest` green (in-memory path, no Redis needed), and `docker compose up --build` green against real containerized Postgres **and** Redis (`test_redis_lobby_store.cpp` passing). `server.cpp` still wires the in-memory path only — switching to Redis at runtime is deliberately deferred to Phase 5 (see Known Limitation below, still applicable).
**Goal:** Move the small, serializable pieces of in-process state — `LobbyRegistry`'s connection/identity map, later `Matchmaker`'s waiting queue and `PlayerRegistry`'s color assignments — into Redis-backed implementations, behind the exact same public interfaces the callers already use.
**Why here:** Only safe because Phase 0 already made these clean classes with clear interfaces — this becomes a technical swap, not a redesign.

**Scope correction (important):** not everything in these classes is a Redis candidate. `GameRegistry::matches_` holds `unique_ptr<GameMatch>` — live, actively-ticking game objects with a background thread driving them (`updateAll`), not inert data. Those cannot be serialized into Redis and keep running; sharing active games across processes is a Phase 5/6 problem (real Game Allocator + shards), not this phase. Only `GameRegistry`'s lookup indices (`usernameToGame_`, `roomCodeToGame_`, both simple `string → GameId`) are in scope here, and only once a pilot on simpler state proves the pattern.

**Pilot: `LobbyRegistry` first.** Smallest and simplest of the four — no dependency on `GameMatch`, no `chrono::time_point` fields to serialize (unlike `Matchmaker`/`PlayerRegistry`, which have timeouts/grace periods to think through separately). Design (mirrors the `PlayerAccountStore` pattern exactly):
- New `LobbyStore` interface (`server/lobby/lobby_store.hpp`): owns only the data (`nextConnectionId`, `addConnection`/`removeConnection`/`hasConnection`, `setIdentity`/`identityFor`). `LobbyRegistry` keeps its own orchestration logic (the `onIdentified` callback) and delegates storage to an injected `LobbyStore&`.
- `InMemoryLobbyStore` — today's behavior, unchanged, becomes the default (`LobbyRegistry`'s no-arg constructor keeps working exactly as-is, so no existing test or call site changes).
- `RedisLobbyStore` (new) — same interface, backed by `sw::redis::Redis`. Connection ids are allocated via Redis `INCR` (not a local counter) specifically so ids stay unique across however many processes share the store — otherwise two Gateway processes would hand out colliding ids.
- `LobbyRegistry(LobbyStore& store)` constructor added alongside the existing default one, so `server.cpp` can opt in later without forcing the change now (same "build it, prove it, wire it in later" approach already used for `PostgresPlayerAccountStore`).

**Decision — hiredis vs. redis-plus-plus (revised after trying both):** started with redis-plus-plus for its higher-level API (same reasoning as pqxx over raw libpq in Phase 2), but its CMake integration does `find_path(HIREDIS_HEADER hiredis)` / `find_library(HIREDIS_LIB hiredis)`, which expects an *installed* hiredis layout (`include/hiredis/hiredis.h`, nested). `FetchContent`'s source tree is the raw repo (flat — `hiredis.h` at the root), so the search failed (`HIREDIS_HEADER-NOTFOUND`) even though hiredis itself built cleanly via `FetchContent`. Rather than work around redis-plus-plus's CMake with a symlink/shim (fragile on Windows), switched to hiredis's raw C API directly — smaller dependency surface, and already proven to build cleanly in the same tree. `RedisLobbyStore` still hides this behind the same `LobbyStore` interface, so nothing outside `redis_lobby_store.cpp` is aware hiredis is a C library.
**Decision — how to build it:** `FetchContent` for `hiredis` only (same pattern as `ixwebsocket`/`sqlite3`), tried on native Windows first — this worked, unlike Phase 2's pqxx, which was forced into the pkg-config/Docker-only route because vcpkg was network-blocked and libpq itself isn't FetchContent-friendly. `hiredis` is a plain CMake GitHub project, and its `FetchContent` build succeeded natively on the first attempt.

**Known limitation — flagged before implementation, not discovered after:** moving `LobbyRegistry` to Redis is *not* by itself sufficient to allow multiple server instances. It only makes one piece of state (who's connected, who's identified) shared. Still unaddressed, and deferred to later phases:
- **Cross-instance coordination** (locks, Pub/Sub, or similar) for anything that today assumes "only one process is deciding" — e.g. `Matchmaker` pairing two waiting players must not double-pair them if two Matchmaker instances both see the same queue. This is Phase 4 (NATS) territory, or a Redis-native mechanism (`WATCH`/Lua scripts, Redis Pub/Sub) evaluated at that point.
- **Active game management across instances** — once `GameMatch` objects themselves need to be reachable from any process (not just their lookup index), that's the real Game Allocator + shard-routing problem (Phase 5/6), not a Redis data swap.
- **In-process callbacks stay in-process.** `LobbyRegistry::onIdentified_` is a `std::function` — it fires only within the process that called `identify()`. A second process reacting to "someone in Gateway A just logged in" needs actual cross-process delivery (Pub/Sub, Phase 4), not this pilot.

This phase's pilot is a correct, low-risk first step — proving the storage-interface pattern extends beyond accounts — but it is explicitly not a complete distributed-architecture solution on its own.

**Decision Log:**
- Step 1 (`LobbyStore`/`InMemoryLobbyStore`) done. New files: `server/lobby/lobby_store.hpp`, `server/lobby/in_memory_lobby_store.hpp/.cpp`. `LobbyRegistry` rewritten to delegate to `LobbyStore&`, with its default constructor now owning an internal `InMemoryLobbyStore` — so `LobbyRegistry lobby;` (used throughout `test_lobby_registry.cpp` and `server.cpp`) keeps compiling and behaving identically; zero test or call-site changes needed. No `CMakeLists.txt` change required — `server/lobby/*.cpp` was already globbed.
- Found and fixed a lock-scope issue while doing this extraction (same class of bug as the earlier mutex audit): the original `LobbyRegistry::identify()` held its mutex for the entire function, including the call to the external `onIdentified_` callback. The rewritten version copies the handler under a brief lock and invokes it outside the lock.
- Connection-id allocation was pulled into `LobbyStore::nextConnectionId()` (rather than staying a `LobbyRegistry`-local counter) specifically so a future `RedisLobbyStore` can make it atomic across processes via Redis `INCR` — a local counter per process would otherwise hand out colliding ids once more than one process shares the store.
- Tests added: one case in `test_lobby_registry.cpp` covering the previously-uncovered `LobbyRegistry(LobbyStore&)` injection constructor, plus a new `server/tests/unit/lobby/test_in_memory_lobby_store.cpp` (8 cases) testing `InMemoryLobbyStore` directly against the `LobbyStore` contract — same precedent as `test_sqlite_player_account_store.cpp` testing its store directly rather than only through `AuthService`. Written so the same contract-level cases can be reused against `RedisLobbyStore` once it exists.
- Step 2 done: `RedisLobbyStore` (`server/lobby/redis_lobby_store.hpp/.cpp`, raw hiredis — see the hiredis-vs-redis-plus-plus note above), `redis` service added to `docker-compose.yml` (image `redis:7-alpine`, `pg_isready`-style healthcheck via `redis-cli ping`), `tests` service now depends on both `postgres` and `redis` being healthy.
- Found and fixed a real bug: `target_include_directories(hiredis PUBLIC ${hiredis_SOURCE_DIR})` (added defensively, to guarantee the include path reached our targets) broke the Docker/Linux build with `CMake Error: Target "hiredis" INTERFACE_INCLUDE_DIRECTORIES property contains path ... which is prefixed in the build directory`. Cause: hiredis's own `export(EXPORT hiredis-targets ...)` call makes `hiredis` an exported target, and CMake forbids an exported target's interface include directories from containing a raw (non-generator-expression) path inside the build tree. hiredis's own `CMakeLists.txt` already does this correctly (`TARGET_INCLUDE_DIRECTORIES(hiredis PUBLIC $<INSTALL_INTERFACE:include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)`), so the defensive line was both redundant and broken — removed entirely. Didn't surface on the native Windows build because `cmake -S ... -B ...` and `cmake --build ...` were run as separate commands there (the Generate-step failure didn't block the later build step, unlike Docker's `&&`-chained `RUN`, which fails hard on the first nonzero exit code).
- Found and fixed a minor build-hygiene issue: hiredis registers its own internal `hiredis-test` (via `test.sh`, a bash script) as a CTest test regardless of the `DISABLE_TESTS` option's expected effect — it isn't runnable through `ctest` on Windows and isn't our code. Disabled explicitly with `set_tests_properties(hiredis-test PROPERTIES DISABLED TRUE)` rather than relying on that option.
- Testing gate: unlike `PQXX_FOUND` (auto-detects whether libpqxx is installed), `hiredis` always builds via `FetchContent` — there's no absence to detect, only a real Redis instance to be reachable or not. Added an explicit `option(REDIS_TESTS ... OFF)` instead; `test_redis_lobby_store.cpp` (new, same shape as `test_postgres_player_account_store.cpp`) is excluded from `TEST_SOURCES` unless `REDIS_TESTS=ON`, so a native `ctest` run (no Redis reachable) isn't broken by it. `server/Dockerfile`'s `cmake` configure step now passes `-DREDIS_TESTS=ON`, so the real integration test only runs there, against the `redis` Compose service.

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
- **2026-07-30** — Phase 3 scoped down and given a concrete pilot plan (`LobbyRegistry` via a new `LobbyStore` interface, `InMemoryLobbyStore`/`RedisLobbyStore`), before any code was written. Added an explicit scope correction (`GameMatch` objects are out of scope for Redis) and a known-limitations note (Redis alone doesn't enable multi-instance serving — cross-instance coordination and active-game distribution are still open, deferred to Phases 4–6).
- **2026-07-30** — Phase 3 pilot completed end to end: `LobbyStore`/`InMemoryLobbyStore`/`RedisLobbyStore` implemented, tested (native + Docker), `redis` service added to `docker-compose.yml`. Along the way: pivoted from redis-plus-plus to raw hiredis (CMake integration issue), disabled hiredis's own stray `hiredis-test`, and fixed a real exported-target include-directory bug that broke the Docker build only (see Phase 3 Decision Log for details).
