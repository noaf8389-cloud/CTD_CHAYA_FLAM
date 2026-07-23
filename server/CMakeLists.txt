cmake_minimum_required(VERSION 3.16)
project(KungFuChessServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG v11.4.5
)
set(USE_TLS OFF CACHE BOOL "" FORCE)
set(USE_ZLIB OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(ixwebsocket)

set(LOGIC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/logic")
file(GLOB_RECURSE LOGIC_SOURCES "${LOGIC_ROOT}/*.cpp")

file(GLOB BUS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/bus/*.cpp")
file(GLOB NETWORK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/network/*.cpp")
file(GLOB SERVER_ROOT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(KungFuChessServer ${SERVER_ROOT_SOURCES} ${LOGIC_SOURCES} ${BUS_SOURCES} ${NETWORK_SOURCES})
target_include_directories(KungFuChessServer PRIVATE ${LOGIC_ROOT} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(KungFuChessServer PRIVATE ixwebsocket)

enable_testing()
file(GLOB_RECURSE TEST_SOURCES "tests/unit/*.cpp")

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
    ${BUS_SOURCES}
)

target_include_directories(KungFuChessServerTests PRIVATE ${LOGIC_ROOT} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(KungFuChessServerTests PRIVATE ixwebsocket)

add_test(NAME KungFuChessServerTests COMMAND KungFuChessServerTests)
