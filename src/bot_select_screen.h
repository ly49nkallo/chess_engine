#ifndef CE_BOT_SELECT_SCREEN_H
#define CE_BOT_SELECT_SCREEN_H

/*
 * bot_select_screen.h -- pre-game screen for choosing White and Black
 * player types (Human or a named bot).
 *
 * Writes the selection into g_game_config before transitioning to GAME.
 */

void bot_select_screen_init(void);
void bot_select_screen_update(void);
void bot_select_screen_draw(void);
void bot_select_screen_unload(void);
int  bot_select_screen_ended(void);

#endif /* CE_BOT_SELECT_SCREEN_H */
