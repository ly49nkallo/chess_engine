#ifndef CE_SCREENS_H
#define CE_SCREENS_H

typedef enum ScreenState {MENU=1, GAME} ScreenState;


// Title Screen
void InitTitleScreen(void);
void UpdateTitleScreen(void);
void DrawTitleScreen(void);
void UnloadTitleScreen(void);
int TitleScreenEnded(void);

// Game Screen
void InitGameScreen(void);
void UpdateGameScreen(void);
void DrawGameScreen(void);
void UnloadGameScreen(void);
int GameScreenEnded(void);


#endif // CE_SCREENS_H