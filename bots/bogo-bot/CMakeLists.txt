cmake_minimum_required(VERSION 3.12)
project(bogo_bot C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(BOGO_BOT
    bot_main.c
    bot_uci.c
    uci_client.c
    bot_logic.c
)

target_include_directories(BOGO_BOT PRIVATE .)

set_target_properties(BOGO_BOT PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bots)

# After building BOGO_BOT, copy it into the engine's output directory
# so the bot_registry can discover it at runtime.
add_custom_command(
    TARGET BOGO_BOT POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:CHESS_ENGINE>/bots
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:BOGO_BOT>
        $<TARGET_FILE_DIR:CHESS_ENGINE>/bots/$<TARGET_FILE_NAME:BOGO_BOT>
    COMMENT "Deploying BOGO_BOT to engine bots/ directory"
)
