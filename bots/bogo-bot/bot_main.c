/*
 * bot_main.c -- entry point for bogo-bot.
 *
 * Two modes:
 *
 *   UCI server mode (used by the chess GUI):
 *     bogo-bot uci <engine_path>
 *     Speaks the UCI protocol on stdin/stdout, delegating move
 *     generation to the chess engine binary at engine_path.
 *
 *   Self-play mode (standalone testing):
 *     bogo-bot <engine_path> [movetime_ms] [max_plies]
 *     Runs a complete game between two engine instances and prints the
 *     move list to stdout.
 */

#include "bot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declaration from bot_uci.c */
void bogo_bot_uci_loop(const char *engine_path);

/* Try a few default engine locations relative to the working directory */
static const char *DEFAULT_ENGINE_PATHS[] = {
#if defined(_WIN32) || defined(_WIN64)
    "CHESS_ENGINE.exe",
    "..\\build\\CHESS_ENGINE\\Debug\\CHESS_ENGINE.exe",
    "..\\build\\CHESS_ENGINE\\CHESS_ENGINE.exe",
#else
    "./CHESS_ENGINE",
    "../build/CHESS_ENGINE/CHESS_ENGINE",
    "../build/CHESS_ENGINE/chess_engine",
#endif
    NULL
};

static const char *find_engine(void)
{
    for (int i = 0; DEFAULT_ENGINE_PATHS[i]; i++) {
        FILE *f = fopen(DEFAULT_ENGINE_PATHS[i], "rb");
        if (f) { fclose(f); return DEFAULT_ENGINE_PATHS[i]; }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    /* -----------------------------------------------------------
     * UCI server mode: bogo-bot uci [engine_path]
     * ----------------------------------------------------------- */
    if (argc >= 2 && strcmp(argv[1], "uci") == 0) {
        const char *engine_path = (argc >= 3) ? argv[2] : find_engine();
        if (!engine_path) {
            fprintf(stderr,
                "[bogo-bot] UCI mode: could not find engine.\n"
                "Usage: bogo-bot uci <engine_path>\n");
            return 1;
        }
        bogo_bot_uci_loop(engine_path);
        return 0;
    }

    /* -----------------------------------------------------------
     * Self-play mode: bogo-bot [engine_path] [movetime_ms] [max_plies]
     * ----------------------------------------------------------- */
    const char *engine_path = (argc > 1) ? argv[1] : find_engine();
    int movetime_ms = (argc > 2) ? atoi(argv[2]) : 100;
    int max_plies   = (argc > 3) ? atoi(argv[3]) : 200;

    if (!engine_path) {
        fprintf(stderr,
            "Usage: bogo-bot <engine_path> [movetime_ms] [max_plies]\n"
            "       bogo-bot uci <engine_path>\n"
            "\nCould not auto-locate chess engine.\n");
        return 1;
    }

    printf("[bogo-bot] Engine: %s  movetime: %d ms  max_plies: %d\n",
           engine_path, movetime_ms, max_plies);

    GameResult result = bot_run_game(engine_path, movetime_ms, max_plies);
    return (result == GAME_ABORTED) ? 1 : 0;
}
