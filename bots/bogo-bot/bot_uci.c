/*
 * bot_uci.c -- UCI server mode for bogo-bot.
 *
 * When invoked as: bogo-bot <engine_path>
 * bogo-bot acts as a UCI engine itself: it reads UCI commands on stdin,
 * delegates move generation to the chess engine subprocess, and writes
 * UCI responses to stdout.
 *
 * This lets the GUI select bogo-bot as a player and communicate with it
 * over a standard UCI pipe, just like any other bot.
 */

#include "bot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Maximum length of the move-history string we build */
#define HISTORY_MAX 4096

void bogo_bot_uci_loop(const char *engine_path)
{
    /* Announce ourselves */
    printf("id name bogo-bot\n");
    printf("id author chess_engine_project\n");
    printf("uciok\n");
    fflush(stdout);

    /* Spawn the engine subprocess once; reuse it for the whole session */
    UciClient *engine = uci_client_create(engine_path);
    if (!engine) {
        fprintf(stderr, "[bogo-bot] Failed to start engine: %s\n", engine_path);
        return;
    }

    /* Track the current position as a move-history string */
    char history[HISTORY_MAX] = "";
    char start_fen[512] = ""; /* empty = startpos */

    char line[1024];
    while (fgets(line, sizeof(line), stdin)) {
        /* Strip newline */
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        char *cr = strchr(line, '\r');
        if (cr) *cr = '\0';

        if (strcmp(line, "uci") == 0) {
            printf("id name bogo-bot\n");
            printf("id author chess_engine_project\n");
            printf("uciok\n");
            fflush(stdout);

        } else if (strcmp(line, "isready") == 0) {
            printf("readyok\n");
            fflush(stdout);

        } else if (strcmp(line, "ucinewgame") == 0) {
            history[0] = '\0';
            start_fen[0] = '\0';
            uci_client_send(engine, "ucinewgame");

        } else if (strncmp(line, "position", 8) == 0) {
            /* Parse and store the position for use in "go" */
            const char *p = line + 8;
            while (*p == ' ') p++;
            history[0] = '\0';
            start_fen[0] = '\0';

            if (strncmp(p, "startpos", 8) == 0) {
                p += 8;
            } else if (strncmp(p, "fen", 3) == 0) {
                p += 3;
                while (*p == ' ') p++;
                /* Copy FEN fields (up to "moves" keyword or EOL) */
                char *f = start_fen;
                char *f_end = start_fen + sizeof(start_fen) - 1;
                while (*p && strncmp(p, "moves", 5) != 0 && f < f_end) {
                    *f++ = *p++;
                }
                /* Trim trailing space */
                while (f > start_fen && *(f-1) == ' ') f--;
                *f = '\0';
            }

            while (*p == ' ') p++;
            if (strncmp(p, "moves", 5) == 0) {
                p += 5;
                while (*p == ' ') p++;
                strncpy(history, p, HISTORY_MAX - 1);
                history[HISTORY_MAX - 1] = '\0';
            }

        } else if (strncmp(line, "go", 2) == 0) {
            /* Build a BotState and ask the engine for a move */
            BotState state;
            bot_init(&state, start_fen[0] ? start_fen : NULL, true);

            /* Replay the history into the BotState */
            char hist_copy[HISTORY_MAX];
            strncpy(hist_copy, history, HISTORY_MAX - 1);
            hist_copy[HISTORY_MAX - 1] = '\0';
            char *tok = strtok(hist_copy, " \t");
            while (tok) {
                /* bot_record_move just appends; we also need to flip turn */
                bot_record_move(&state, tok);
                tok = strtok(NULL, " \t");
            }

            /* Ask the engine */
            uci_client_send_position(engine, &state);
            char move[8];
            bool has_move = uci_client_go_get_bestmove(engine, 100, move);

            if (!has_move) {
                printf("bestmove 0000\n");
            } else {
                printf("bestmove %s\n", move);
            }
            fflush(stdout);

        } else if (strcmp(line, "stop") == 0) {
            /* Nothing to stop in this synchronous implementation */
        } else if (strcmp(line, "quit") == 0) {
            break;
        }
    }

    uci_client_destroy(engine);
}
