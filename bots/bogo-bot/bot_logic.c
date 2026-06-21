/*
 * bot_logic.c -- game-level logic and random move selection.
 *
 * "Random legal" strategy: for the bot's own turn we ask the engine for
 * its best move (the engine picks the first legal move it finds), but to
 * add variety we could also implement independent random selection here.
 *
 * For now the simplest correct thing is:
 *   - both sides just ask the engine with "go movetime <N>"
 *   - the engine returns a legal move (first-legal for now)
 *   - the bot records it and loops
 *
 * This file is intentionally kept thin so that future search / evaluation
 * code can be layered in without changing the rest of the architecture.
 */

#include "bot.h"
#include <time.h>

/* ------------------------------------------------------------------
 * bot_init
 * ------------------------------------------------------------------ */
void bot_init(BotState *state, const char *start_fen, bool bot_is_white)
{
    memset(state, 0, sizeof(*state));
    state->start_fen    = start_fen;
    state->bot_is_white = bot_is_white;
    state->white_to_move = true; /* white always goes first */
}

/* ------------------------------------------------------------------
 * bot_record_move
 * ------------------------------------------------------------------ */
void bot_record_move(BotState *state, const char *move)
{
    if (state->move_count >= BOT_MAX_MOVES) {
        fprintf(stderr, "[bot] WARNING: move history full, discarding oldest\n");
        /* Shift the window (crude but keeps things simple) */
        memmove(state->moves[0], state->moves[1],
                (size_t)(BOT_MAX_MOVES - 1) * sizeof(state->moves[0]));
        state->move_count = BOT_MAX_MOVES - 1;
    }
    strncpy(state->moves[state->move_count], move,
            sizeof(state->moves[0]) - 1);
    state->moves[state->move_count][sizeof(state->moves[0]) - 1] = '\0';
    state->move_count++;
    state->white_to_move = !state->white_to_move;
    state->half_move_clock++;
}

/* ------------------------------------------------------------------
 * bot_run_game
 *
 * Run a complete self-play game (engine vs engine).
 * Returns the game result.
 * ------------------------------------------------------------------ */
GameResult bot_run_game(const char *engine_path, int movetime_ms,
                        int max_plies)
{
    printf("[bot] Starting game.  Engine: %s  movetime: %d ms\n",
           engine_path, movetime_ms);

    UciClient *client = uci_client_create(engine_path);
    if (!client) {
        fprintf(stderr, "[bot] Failed to start engine: %s\n", engine_path);
        return GAME_ABORTED;
    }

    /* Reset engine state for a clean game */
    uci_client_send(client, "ucinewgame");

    BotState state;
    bot_init(&state, NULL, true); /* bot plays white; engine plays both */

    GameResult result = GAME_ONGOING;
    int ply = 0;

    while (result == GAME_ONGOING) {
        if (max_plies > 0 && ply >= max_plies) {
            printf("[bot] Reached max plies (%d); declaring draw.\n", max_plies);
            result = GAME_DRAW;
            break;
        }

        /* Tell the engine the current position */
        uci_client_send_position(client, &state);

        /* Ask for a move */
        char move[8];
        bool has_move = uci_client_go_get_bestmove(client, movetime_ms, move);

        if (!has_move) {
            /* "0000" -- no legal move available */
            printf("[bot] No legal move for %s -- game over.\n",
                   state.white_to_move ? "White" : "Black");
            /*
             * We can't distinguish checkmate from stalemate here without
             * additional check detection.  Report as a simple game over;
             * a future improvement is to query is_in_check via a separate
             * mechanism.
             */
            result = state.white_to_move ? GAME_BLACK_WINS : GAME_WHITE_WINS;
            /* Could also be stalemate -> GAME_DRAW; treat as loss for now */
            break;
        }

        const char *side = state.white_to_move ? "White" : "Black";
        printf("[bot] %s plays: %s   (ply %d)\n", side, move, ply + 1);

        bot_record_move(&state, move);
        ply++;
    }

    /* Print result */
    switch (result) {
    case GAME_WHITE_WINS: printf("[bot] Result: White wins\n");   break;
    case GAME_BLACK_WINS: printf("[bot] Result: Black wins\n");   break;
    case GAME_DRAW:       printf("[bot] Result: Draw\n");         break;
    case GAME_ABORTED:    printf("[bot] Result: Aborted\n");      break;
    default:              printf("[bot] Result: Unknown\n");      break;
    }

    /* Print the full move list */
    printf("[bot] Move list (%d plies):", state.move_count);
    for (int i = 0; i < state.move_count; i++) {
        if (i % 2 == 0) printf("  %d.", i / 2 + 1);
        printf(" %s", state.moves[i]);
    }
    printf("\n");

    uci_client_destroy(client);
    return result;
}
