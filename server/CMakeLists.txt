cmake_minimum_required(VERSION 3.16)
project(KungFuChessServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# hiredis always builds (no external system dependency to detect, unlike libpqxx),
# but a real Redis instance is only reachable in the Docker Compose environment —
# so its integration tests are gated behind this explicit flag, not auto-detection.
option(REDIS_TESTS "Build integration tests that require a real Redis instance" OFF)

include(FetchContent)
FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG v11.4.5
)
set(USE_TLS OFF CACHE BOOL "" FORCE)
set(USE_ZLIB OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(ixwebsocket)

FetchContent_Declare(
    sqlite3
    GIT_REPOSITORY https://github.com/azadkuh/sqlite-amalgamation.git
    GIT_TAG 3.36.0
)
FetchContent_MakeAvailable(sqlite3)
set(SQLITE3_ROOT ${sqlite3_SOURCE_DIR})

FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG v1.2.0
)
set(DISABLE_TESTS ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(hiredis)

# hiredis registers its own "hiredis-test" via test.sh regardless of DISABLE_TESTS
# on some versions; that script isn't runnable through ctest on Windows and isn't
# our code anyway — disable it explicitly rather than let it show up as a failure.
if(TEST hiredis-test)
    set_tests_properties(hiredis-test PROPERTIES DISABLED TRUE)
endif()

set(LOGIC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/logic")
set(SHARED_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../shared")
file(GLOB_RECURSE LOGIC_SOURCES "${LOGIC_ROOT}/*.cpp")

file(GLOB SHARED_BUS_SOURCES "${SHARED_ROOT}/bus/*.cpp")
file(GLOB SHARED_LOGGING_SOURCES "${SHARED_ROOT}/logging/*.cpp")
file(GLOB NETWORK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/network/*.cpp")
file(GLOB SERVER_ROOT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB DB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/db/*.cpp")
file(GLOB RATING_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/rating/*.cpp")
file(GLOB MATCHES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/matches/*.cpp")
file(GLOB LOBBY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lobby/*.cpp")
file(GLOB AUTH_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/auth/*.cpp")

find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(PQXX libpqxx)
endif()

file(GLOB_RECURSE TEST_SOURCES "tests/unit/*.cpp")
if(NOT PQXX_FOUND)
    list(REMOVE_ITEM DB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/db/postgres_player_account_store.cpp")
    list(REMOVE_ITEM TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/DB/test_postgres_player_account_store.cpp")
endif()
if(NOT REDIS_TESTS)
    list(REMOVE_ITEM TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/lobby/test_redis_lobby_store.cpp")
endif()

add_executable(KungFuChessServer ${SERVER_ROOT_SOURCES} ${LOGIC_SOURCES} ${SHARED_BUS_SOURCES} ${SHARED_LOGGING_SOURCES} ${NETWORK_SOURCES} ${DB_SOURCES} ${RATING_SOURCES} ${MATCHES_SOURCES} ${LOBBY_SOURCES} ${AUTH_SOURCES} ${SQLITE3_ROOT}/sqlite3.c)
target_include_directories(KungFuChessServer PRIVATE ${LOGIC_ROOT} ${SHARED_ROOT} ${CMAKE_CURRENT_SOURCE_DIR} ${SQLITE3_ROOT})
target_link_libraries(KungFuChessServer PRIVATE ixwebsocket hiredis)

if(PQXX_FOUND)
    target_include_directories(KungFuChessServer PRIVATE ${PQXX_INCLUDE_DIRS})
    target_link_libraries(KungFuChessServer PRIVATE ${PQXX_LIBRARIES})
endif()

enable_testing()

set(SERVER_ROOT_SOURCES_FOR_TESTS ${SERVER_ROOT_SOURCES})
list(REMOVE_ITEM SERVER_ROOT_SOURCES_FOR_TESTS
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/server.cpp"
)

add_executable(KungFuChessServerTests
    tests/test_main.cpp
    tests/catch2/catch_amalgamated.cpp
    ${NETWORK_SOURCES}
    ${SERVER_ROOT_SOURCES_FOR_TESTS}
    ${TEST_SOURCES}
    ${LOGIC_SOURCES}
    ${SHARED_BUS_SOURCES}
    ${SHARED_LOGGING_SOURCES}
    ${DB_SOURCES}
    ${RATING_SOURCES}
    ${MATCHES_SOURCES}
    ${LOBBY_SOURCES}
    ${AUTH_SOURCES}
    ${SQLITE3_ROOT}/sqlite3.c
)

target_include_directories(KungFuChessServerTests PRIVATE ${LOGIC_ROOT} ${SHARED_ROOT} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/tests ${SQLITE3_ROOT})
target_link_libraries(KungFuChessServerTests PRIVATE ixwebsocket hiredis)

if(PQXX_FOUND)
    target_include_directories(KungFuChessServerTests PRIVATE ${PQXX_INCLUDE_DIRS})
    target_link_libraries(KungFuChessServerTests PRIVATE ${PQXX_LIBRARIES})
endif()

add_test(NAME KungFuChessServerTests COMMAND KungFuChessServerTests)
