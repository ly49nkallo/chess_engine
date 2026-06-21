cmake_minimum_required(VERSION 3.12)
project(chess_bot C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(CHESS_BOT
    bot_main.c
    uci_client.c
    bot_logic.c
)

target_include_directories(CHESS_BOT PRIVATE .)

if(WIN32)
    # No extra libs needed on Windows for CreateProcess / pipes
else()
    # POSIX: nothing extra needed for fork/pipe
endif()

set_target_properties(CHESS_BOT PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/CHESS_BOT)
