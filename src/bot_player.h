#ifndef CE_BOT_PLAYER_H
#define CE_BOT_PLAYER_H

/*
 * bot_player.h -- thin wrapper that drives a bot executable as a UCI
 * subprocess.  Used by game_screen.c to let a bot make moves inside the
 * GUI game loop.
 *
 * The subprocess is spawned once per game and kept alive until the game
 * ends.  Moves are requested with bot_player_request_move() and collected
 * with bot_player_poll_move().  These calls are non-blocking so they do
 * not stall the 60 fps render loop.
 */

#include "chess_engine.h"
#include <stdbool.h>

#define BP_MOVE_MAX 8   /* max length of a UCI move string incl. NUL */

/* Opaque handle for one bot subprocess. */
typedef struct BotPlayer BotPlayer;

/*
 * bot_player_create -- spawn the bot at exe_path with the engine binary
 * at engine_path as its argument (the bot talks UCI back to THIS engine).
 * Returns NULL on failure.
 *
 * exe_path    : path to the bot executable  (e.g. "bots/bogo-bot.exe")
 * engine_path : path to the chess engine    (e.g. "CHESS_ENGINE.exe")
 */
BotPlayer *bot_player_create(const char *exe_path, const char *engine_path);

/*
 * bot_player_destroy -- terminate the subprocess and free resources.
 */
void bot_player_destroy(BotPlayer *bp);

/*
 * bot_player_new_game -- send "ucinewgame" and reset move history.
 */
void bot_player_new_game(BotPlayer *bp);

/*
 * bot_player_request_move -- send the current position and "go movetime N"
 * to the bot.  Does NOT wait for the response; call bot_player_poll_move()
 * on subsequent frames to collect it.
 *
 * board       : current game position
 * move_history: UCI move strings played so far (space-separated), may be NULL
 * start_fen   : starting FEN, or NULL for the standard position
 * movetime_ms : milliseconds the bot may think
 */
void bot_player_request_move(BotPlayer *bp,
                              const char *move_history,
                              const char *start_fen,
                              int movetime_ms);

/*
 * bot_player_poll_move -- check whether the bot has replied.
 * If a "bestmove" line is available, writes the UCI move into move_out
 * (must have room for BP_MOVE_MAX bytes) and returns true.
 * Returns false and leaves move_out untouched if no answer yet.
 * Returns true with move_out == "0000" when the bot has no legal move.
 */
bool bot_player_poll_move(BotPlayer *bp, char *move_out);

/*
 * bot_player_is_thinking -- returns true if a move was requested but not
 * yet collected.
 */
bool bot_player_is_thinking(BotPlayer *bp);

#endif /* CE_BOT_PLAYER_H */
