#ifndef CE_GAME_SCREEN_H
#define CE_GAME_SCREEN_H

#include "raylib.h"
#include "screens.h"
#include "chess_engine.h"
#include "utilities.h"

#include <stdlib.h>
#include <stdio.h>

#define LOW_RES_SPRITESHEET_PATH "resources/640px-Chess_Pieces_Sprite.png"
#define HIGH_RES_SPRITESHEET_PATH "resources/1024px-Chess_Pieces_Sprite.png"

struct BoardTheme {
    Color backgroundColor;
    Color boardColorDark;
    Color boardColorLight;
    Color selectedColor; // when you left click a piece
    Color highlightColor; // when you right click a tile
    Color arrowColor;
    char* pieceFontName; // default, neo, ...
};

/* METHODS */
void game_screen_init(void);
void game_screen_update(void);
void render_tiles(void);
void render_labels(void);
void render_pieces(ChessBoard* board);
void game_screen_draw(void);
int game_screen_ended(void);

#endif CE_GAME_SCREEN_H