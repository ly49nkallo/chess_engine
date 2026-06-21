#ifndef BOT_H
#define BOT_H

/*
 * bot.h -- shared types for the chess bot that communicates with the
 *          chess engine via the UCI protocol over stdin/stdout pipes.
 *
 * Architecture overview:
 *   CHESS_BOT (this program)
 *       |  stdin/stdout pipes (UCI protocol)
 *       v
 *   CHESS_ENGINE  (chess_engine uci)
 *
 * The bot owns the game loop:
 *   1. It sends "position" + "go" to the engine.
 *   2. It reads back "bestmove <move>".
 *   3. For the bot's own side it picks a random legal move (basic strength).
 *   4. The engine side always follows the engine's own choice.
 *
 * Both sides currently go through the engine for move selection, so this
 * also works as an engine-vs-engine self-play harness.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Maximum length of a single UCI line */
#define UCI_LINE_MAX 1024

/* Maximum number of half-moves in the move history */
#define BOT_MAX_MOVES 512

/* Result of a game */
typedef enum {
    GAME_ONGOING = 0,
    GAME_WHITE_WINS,
    GAME_BLACK_WINS,
    GAME_DRAW,
    GAME_ABORTED
} GameResult;

/*
 * UciClient -- opaque handle wrapping the engine subprocess.
 * Defined fully in uci_client.c.
 */
typedef struct UciClient UciClient;

/*
 * BotState -- game-level state maintained by the bot.
 */
typedef struct BotState {
    /* Ordered list of half-moves played so far, in UCI notation (e.g. "e2e4") */
    char moves[BOT_MAX_MOVES][8];
    int  move_count;

    /* Starting FEN; if NULL the standard start position is used */
    const char *start_fen;

    /* Which colour the *bot* controls (true = white, false = black).
     * The other side is also driven by the engine (engine-vs-engine). */
    bool bot_is_white;

    /* Whose turn it is right now */
    bool white_to_move;

    /* Number of half-moves played (used for 50-move tracking) */
    int half_move_clock;
} BotState;

/* ------------------------------------------------------------------ */
/* uci_client.c API                                                     */
/* ------------------------------------------------------------------ */

/*
 * uci_client_create -- spawn the engine executable and perform the UCI
 * handshake (uci / uciok / isready / readyok).
 *
 * engine_path: path to the compiled chess_engine binary.
 * Returns a heap-allocated UciClient on success, NULL on failure.
 */
UciClient *uci_client_create(const char *engine_path);

/*
 * uci_client_destroy -- terminate the engine process and free resources.
 */
void uci_client_destroy(UciClient *client);

/*
 * uci_client_send -- write a UCI command line to the engine.
 * A newline is appended automatically.
 */
void uci_client_send(UciClient *client, const char *cmd);

/*
 * uci_client_readline -- read one line from the engine (blocking).
 * buf must be at least UCI_LINE_MAX bytes.
 * Returns true on success, false on EOF / error.
 */
bool uci_client_readline(UciClient *client, char *buf);

/*
 * uci_client_send_position -- send "position startpos moves ..." or
 * "position fen <fen> moves ..." based on the BotState.
 */
void uci_client_send_position(UciClient *client, const BotState *state);

/*
 * uci_client_go_get_bestmove -- send "go movetime <ms>" and block until
 * the engine responds with "bestmove".  The move is written into move_out
 * (must have room for 6 bytes including NUL).
 * Returns true if a real move was found, false for "0000" (no legal move).
 */
bool uci_client_go_get_bestmove(UciClient *client, int movetime_ms,
                                 char *move_out);

/* ------------------------------------------------------------------ */
/* bot_logic.c API                                                      */
/* ------------------------------------------------------------------ */

/*
 * bot_init -- initialise a BotState for a new game.
 *
 * start_fen: starting position FEN, or NULL for the standard position.
 * bot_is_white: the colour the bot controls.
 */
void bot_init(BotState *state, const char *start_fen, bool bot_is_white);

/*
 * bot_record_move -- append a move (UCI notation) to the move history
 * and flip the side-to-move.
 */
void bot_record_move(BotState *state, const char *move);

/*
 * bot_run_game -- run a complete game between two engine instances
 * (both sides driven by the same engine), printing each move to stdout.
 *
 * engine_path: path to the chess_engine binary.
 * movetime_ms: milliseconds the engine may think per move.
 * max_plies:   stop after this many half-moves (0 = unlimited).
 */
GameResult bot_run_game(const char *engine_path, int movetime_ms,
                        int max_plies);

#endif /* BOT_H */
