#ifndef CE_SCREENS_H
#define CE_SCREENS_H

typedef enum ScreenState {MENU=1, GAME} ScreenState;


// Title Screen
void title_screen_init(void);
void title_screen_update(void);
void title_screen_draw(void);
void title_screen_unload(void);
int title_screen_ended(void);

// Game Screen
void game_screen_init(void);
void game_screen_update(void);
void game_screen_draw(void);
void game_screen_unload(void);
int game_screen_ended(void);


#endif // CE_SCREENS_H