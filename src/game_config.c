#include "game_config.h"
#include <string.h>

GameConfig g_game_config;

void game_config_reset(void)
{
    memset(&g_game_config, 0, sizeof(g_game_config));
    g_game_config.white.type = PLAYER_HUMAN;
    g_game_config.black.type = PLAYER_HUMAN;
}
