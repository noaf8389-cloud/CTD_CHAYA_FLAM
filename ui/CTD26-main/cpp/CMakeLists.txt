cmake_minimum_required(VERSION 3.16)
project(KungFuChess)

# Set C++ standard
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

# Find all source files in src directory
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")

set(LOGIC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../server/logic")
set(LOGIC_SOURCES
    "${LOGIC_ROOT}/io/board_parser.cpp"
    "${LOGIC_ROOT}/rules/piece_rules.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${LOGIC_SOURCES})

# Set OpenCV paths
set(OPENCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/OpenCV_451")
set(OPENCV_INCLUDE_DIR "${OPENCV_DIR}/include")
set(OPENCV_LIB_DIR "${OPENCV_DIR}/bin")

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENCV_INCLUDE_DIR} ${LOGIC_ROOT})

# Link directories
target_link_directories(${PROJECT_NAME} PRIVATE ${OPENCV_LIB_DIR})

# Link OpenCV libraries - use different libraries for Debug vs Release
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:${OPENCV_LIB_DIR}/opencv_world451d.lib>
    $<$<CONFIG:Release>:${OPENCV_LIB_DIR}/opencv_world451.lib>
    $<$<CONFIG:RelWithDebInfo>:${OPENCV_LIB_DIR}/opencv_world451.lib>
    $<$<CONFIG:MinSizeRel>:${OPENCV_LIB_DIR}/opencv_world451.lib>
    ixwebsocket
)

# Copy OpenCV DLLs to output directory
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:Debug>:${OPENCV_LIB_DIR}/opencv_world451d.dll>
        $<$<CONFIG:Release>:${OPENCV_LIB_DIR}/opencv_world451.dll>
        $<$<CONFIG:RelWithDebInfo>:${OPENCV_LIB_DIR}/opencv_world451.dll>
        $<$<CONFIG:MinSizeRel>:${OPENCV_LIB_DIR}/opencv_world451.dll>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

# Print found sources for debugging
message(STATUS "Found source files: ${SOURCES}")
message(STATUS "Found header files: ${HEADERS}") 




enable_testing()

file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")
set(LIB_SOURCES ${SOURCES})
list(REMOVE_ITEM LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(KungFuChessTests ${TEST_SOURCES} ${LIB_SOURCES} ${HEADERS} ${LOGIC_SOURCES})

target_include_directories(KungFuChessTests PRIVATE ${OPENCV_INCLUDE_DIR} ${LOGIC_ROOT})
target_link_directories(KungFuChessTests PRIVATE ${OPENCV_LIB_DIR})

target_link_libraries(KungFuChessTests PRIVATE
    $<$<CONFIG:Debug>:${OPENCV_LIB_DIR}/opencv_world451d.lib>
    $<$<CONFIG:Release>:${OPENCV_LIB_DIR}/opencv_world451.lib>
    $<$<CONFIG:RelWithDebInfo>:${OPENCV_LIB_DIR}/opencv_world451.lib>
    $<$<CONFIG:MinSizeRel>:${OPENCV_LIB_DIR}/opencv_world451.lib>
    ixwebsocket
)

if(WIN32)
    add_custom_command(TARGET KungFuChessTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:Debug>:${OPENCV_LIB_DIR}/opencv_world451d.dll>
        $<$<CONFIG:Release>:${OPENCV_LIB_DIR}/opencv_world451.dll>
        $<$<CONFIG:RelWithDebInfo>:${OPENCV_LIB_DIR}/opencv_world451.dll>
        $<$<CONFIG:MinSizeRel>:${OPENCV_LIB_DIR}/opencv_world451.dll>
        $<TARGET_FILE_DIR:KungFuChessTests>)
endif()

add_test(NAME KungFuChessTests COMMAND KungFuChessTests)
