/*
 * bot_main.c -- entry point for the chess bot.
 *
 * Usage:
 *   CHESS_BOT [engine_path] [movetime_ms] [max_plies]
 *
 *   engine_path   : path to the chess_engine binary  (default: ../build/CHESS_ENGINE/chess_engine)
 *   movetime_ms   : milliseconds per move            (default: 100)
 *   max_plies     : stop after N half-moves, 0 = unlimited (default: 200)
 *
 * Example (from repo root, after building everything):
 *   ./build/CHESS_BOT/CHESS_BOT ./build/CHESS_ENGINE/chess_engine 100 200
 *
 * The bot drives both sides through the same engine instance and prints
 * every move to stdout, making it easy to verify the UCI pipe is working.
 *
 * Exit codes:
 *   0  game completed normally
 *   1  engine failed to start or pipe error
 */

#include "bot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Try a few default engine locations relative to the binary's working dir */
static const char *DEFAULT_ENGINE_PATHS[] = {
#if defined(_WIN32) || defined(_WIN64)
    "..\\build\\CHESS_ENGINE\\Debug\\CHESS_ENGINE.exe",
    "..\\build\\CHESS_ENGINE\\CHESS_ENGINE.exe",
    ".\\CHESS_ENGINE.exe",
    "CHESS_ENGINE.exe",
#else
    "../build/CHESS_ENGINE/chess_engine",
    "../build/CHESS_ENGINE/CHESS_ENGINE",
    "./chess_engine",
    "chess_engine",
#endif
    NULL
};

int main(int argc, char **argv)
{
    const char *engine_path = NULL;
    int movetime_ms = 100;
    int max_plies   = 200;

    /* Parse optional arguments */
    if (argc > 1) engine_path  = argv[1];
    if (argc > 2) movetime_ms  = atoi(argv[2]);
    if (argc > 3) max_plies    = atoi(argv[3]);

    /* Auto-discover engine if not specified */
    if (!engine_path) {
        for (int i = 0; DEFAULT_ENGINE_PATHS[i]; i++) {
            FILE *f = fopen(DEFAULT_ENGINE_PATHS[i], "rb");
            if (f) {
                fclose(f);
                engine_path = DEFAULT_ENGINE_PATHS[i];
                break;
            }
        }
    }

    if (!engine_path) {
        fprintf(stderr,
            "Usage: %s <engine_path> [movetime_ms] [max_plies]\n"
            "\n"
            "Could not auto-locate the chess engine binary.\n"
            "Please build it first (cmake --build build --target CHESS_ENGINE)\n"
            "and pass the path as the first argument.\n",
            argv[0]);
        return 1;
    }

    printf("[bot] Engine: %s\n", engine_path);
    printf("[bot] Movetime: %d ms per move\n", movetime_ms);
    printf("[bot] Max plies: %s\n",
           max_plies ? "unlimited" : "200 (default)");

    GameResult result = bot_run_game(engine_path, movetime_ms, max_plies);

    return (result == GAME_ABORTED) ? 1 : 0;
}
