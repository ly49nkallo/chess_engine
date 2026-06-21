#ifndef CE_GAME_CONFIG_H
#define CE_GAME_CONFIG_H

/*
 * game_config.h -- configuration chosen in the bot-select screen that is
 * forwarded to the game screen before it initialises.
 *
 * The data is stored in a single global instance so that the existing
 * screen-transition machinery (which calls init/update/draw/unload with no
 * arguments) can share it without passing pointers through main.c.
 */

#include "bot_registry.h"
#include <stdbool.h>

typedef enum PlayerType {
    PLAYER_HUMAN = 0,
    PLAYER_BOT
} PlayerType;

typedef struct PlayerConfig {
    PlayerType  type;
    char        bot_exe[BOT_PATH_MAX]; /* path to bot executable; unused for PLAYER_HUMAN */
    char        bot_name[BOT_NAME_MAX];
} PlayerConfig;

typedef struct GameConfig {
    PlayerConfig white;
    PlayerConfig black;
} GameConfig;

/* Single global instance -- written by bot_select_screen, read by game_screen. */
extern GameConfig g_game_config;

/* Reset to two human players. */
void game_config_reset(void);

#endif /* CE_GAME_CONFIG_H */
