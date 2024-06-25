#include "raylib.h"
#include "screens.h"
#include "chess_engine.h"
#include "utilities.h"

#include <stdlib.h>
#include <stdio.h>

#define SPRITE_WIDTH 45.0F

/* LOCAL VARIABLES */
static struct BoardTheme {
    Color backgroundColor;
    Color boardColorDark;
    Color boardColorLight;
    char* pieceFontName; // default, neo, ...
};
static long frameCount;
static int screenEnded;
static struct BoardTheme* currentBoardTheme;
static ChessBoard* currentBoard;
static int boardHeight, boardWidth;
static Vector2 boardPosition;
static Vector2 tileSize;
static int whiteSideDown;
static Font boardTextFont;
static Texture2D spritesheet;

/* METHODS */
void game_screen_init(void);
void game_screen_update(void);
void render_tiles(void);
void render_labels(void);
void render_pieces(ChessBoard* board);
void game_screen_draw(void);
int game_screen_ended(void);

/// @brief performed when game screen is transitioned to
void game_screen_init(void) 
{
    screenEnded = 0;
    frameCount = 0L;
    // setup default board theme
    currentBoardTheme=MemAlloc(sizeof(struct BoardTheme));
    currentBoardTheme->backgroundColor = RAYWHITE;
    currentBoardTheme->boardColorDark = GRAY;
    currentBoardTheme->boardColorLight = LIGHTGRAY;
    currentBoardTheme->pieceFontName = "philosopher_bold";

    whiteSideDown = 1; //Start with white side down

    boardHeight = 8 * GetScreenHeight() / 10; // board will be 80% of the height of the screen
    boardWidth = boardHeight; // Board is a square
    boardPosition.x = (float)(GetScreenWidth() / 2 - boardWidth / 2); // Centred Board Position
    boardPosition.y = (float)(GetScreenHeight() / 2 - boardHeight / 2);

    boardTextFont = LoadFontEx("resources/Philosopher-Bold.ttf", 20, 0, 250); // for the annotations on the board

    spritesheet = LoadTexture("resources/Chess_Pieces_Sprite.png");
    printf("INFO: Spritesheet Loaded Sucessfully\n");

    currentBoard = MemAlloc(sizeof(ChessBoard));
    if (currentBoard == NULL) throw_error(__LINE__, __FILE__, "Not enough RAM");
    chess_board_init(currentBoard);
    generate_board_from_FEN(currentBoard, CE_FEN_STARTING_POSITION);

    printf("INFO: Loaded Game Screen Successfully\n");
}
/// @brief performed once per frame
void game_screen_update(void) 
{
    frameCount++;
}
/// @brief render the colored tiles onto the screen
void render_tiles(void) 
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int tileWidth = boardWidth / 8;
    int i, j;
    for (i = 0; i < 8; i++) { // 1 to 8
        for (j = 0; j < 8; j++) { // A to G
            DrawRectangle(
                (int)(boardPosition.x) + i * tileWidth, 
                (int)(boardPosition.y) + j * tileWidth,
                tileWidth, tileWidth,
                (((i + j) & 1) == whiteSideDown) ? currentBoardTheme->boardColorLight 
                                        : currentBoardTheme->boardColorDark
            );
        }
    }
}
/// @brief render the side labels (1-8, A-H) onto the screen
void render_labels(void) 
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int tileWidth = boardWidth / 8;
    char* ranks = "12345678";
    char* files = "abcdefgh";
    char* symb = MemAlloc(sizeof(char) * 2);
    if (symb == (void *)0)
    {
        printf("ERROR: Could not allocate memory");
        exit(1);
    }
    Vector2 symbolPosition;
    // ranks
    int i;
    for (i = 0; i < 8; i++) {
        *symb = (char)*(ranks + (7 - i));
        *(symb + 1) = '\0'; // terminate string
        Vector2 symbDims = MeasureTextEx(boardTextFont, symb, 20, 5); 
        symbolPosition.x = boardPosition.x - tileWidth/2 - symbDims.x/2;
        symbolPosition.y = boardPosition.y + tileWidth/2 - symbDims.y/2 + (tileWidth * i);
        symbolPosition.y += tileWidth/8; // Small Correction Needed For Some Reason ...
        if ((symbolPosition.x <= 0 || symbolPosition.x >= screenWidth)
            || (symbolPosition.y <= 0 || symbolPosition.y >= screenHeight))
        {
            printf("ERROR: Symbol Position is out of bounds POS:(%f, %f)\n", symbolPosition.x, symbolPosition.y);
            exit(1);
        }
        DrawTextEx(boardTextFont, symb, symbolPosition, 20, 5, BLACK);
    }
    // files
    for (i = 0; i < 8; i++) {
        *symb = (char)*(files + i);
        *(symb + 1) = '\0'; // terminate string
        Vector2 symbDims = MeasureTextEx(boardTextFont, symb, 20, 5); 
        symbolPosition.x = boardPosition.x - tileWidth/2 - symbDims.x/2 + (tileWidth * i) + tileWidth;
        symbolPosition.y = boardPosition.y + boardHeight + tileWidth/2 - symbDims.y/2 ;
        symbolPosition.y += tileWidth/8; // Small Correction Needed For Some Reason ...
        // printf("DEBUG: Board Position: (%f, %f)", symbolPosition.x, symbolPosition.y);
        if ((symbolPosition.x <= 0 || symbolPosition.x >= screenWidth)
            || (symbolPosition.y <= 0 || symbolPosition.y >= screenHeight))
        {
            throw_error(__LINE__, __FILE__, "ERROR: Symbol Position is out of bounds POS:(%f, %f)\n", symbolPosition.x, symbolPosition.y);
        }
        DrawTextEx(boardTextFont, symb, symbolPosition, 20, 5, BLACK);
    }

    MemFree(symb);
}
/// @brief render the game pieces onto the screen
/// @param board 
void render_pieces(ChessBoard* board) 
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int tileWidth = boardWidth / 8;
    Rectangle source = {0, 0, 200, 200};
    const Vector2 origin = {0, 0};
    const float rotation = 0;
    const Color tint = WHITE;
    int i, j;
    for (i = 0; i < 8; i ++) { // rank
        for (j = 0; j < 8; j ++) { // file
            Rectangle dest = {
                boardPosition.x + (float)(j * tileWidth), 
                boardPosition.y + (float)(i * tileWidth), 
                (float)tileWidth, (float)tileWidth
            };
            int piece = board->piece_list[ID_FROM_RANK_FILE(i, j)];
            int color = ((piece & 0b11000) == TILE_WHITE) ? 0 : 1; // 0: White, 1: Black
            /*
            01, 02, 03, 04, 05, 06
            07, 08, 09, 10, 11, 12
            */
            int idx1 = 0;
            switch ((piece & 0b111)) {
                case PAWN: idx1 = 6; break;
                case KNIGHT: idx1 = 4; break;
                case BISHOP: idx1 = 3; break;
                case ROOK: idx1 = 5; break;
                case QUEEN: idx1 = 2; break;
                case KING: idx1 = 1; break;
            };
            if (idx1) {
                int idx2 = idx1 + (6 * color);
                Rectangle source = {
                    (idx1 - 1) * SPRITE_WIDTH, 
                    (1 - color) * SPRITE_WIDTH, 
                    SPRITE_WIDTH, 
                    SPRITE_WIDTH
                };
                DrawTexturePro(spritesheet, source, dest, origin, rotation, tint);
            }
        }
    }
}
/// @brief Render logic for game screen. Performed once per frame
void game_screen_draw(void)
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    DrawRectangle(0, 0, screenWidth, screenHeight, RAYWHITE); // background
    render_tiles();
    render_labels();
    render_pieces(currentBoard);

}
void game_screen_unload(void) 
{
    UnloadFont(boardTextFont);
    MemFree(currentBoardTheme);
    chess_board_destroy(currentBoard);
    MemFree(currentBoard);
}
int game_screen_ended(void)
{
    return screenEnded;
}
