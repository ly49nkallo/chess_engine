#include "raylib.h"
#include "stdlib.h"
#include "screens.h"

static long frameCount;
static int gameScreenEnded;
struct BoardTheme {
    Color backgroundColor;
    Color boardColorDark;
    Color boardColorLight;
    char* pieceFontName; // default, neo, ...
};
struct BoardTheme *currentBoardTheme;
static int boardHeight, boardWidth;
static Vector2 boardPosition;
static Vector2 tileSize;
static int whiteSideDown;
static Font boardTextFont;

void InitGameScreen(void) 
{
    // setup default board theme
    currentBoardTheme=MemAlloc(sizeof(struct BoardTheme));
    currentBoardTheme->backgroundColor = RAYWHITE;
    currentBoardTheme->boardColorDark = GRAY;
    currentBoardTheme->boardColorLight = LIGHTGRAY;
    currentBoardTheme->pieceFontName = "default";

    whiteSideDown = 1; //Start with white side down

    boardHeight = 8 * GetScreenHeight() / 10; // board will be 80% of the height of the screen
    boardWidth = boardHeight ; // Board is a square
    boardPosition.x = GetScreenWidth() / 2 - boardWidth / 2; // Centred Board Position
    boardPosition.y = GetScreenHeight() / 2 - boardHeight / 2;

    boardTextFont = LoadFontEx("resources/Philosopher-Bold.ttf", 20, 0, 250); // for the annotations on the board
}

void UpdateGameScreen(void) 
{

}

void DrawGameScreen(void) 
{
    int tileWidth = boardWidth / 8;
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), RAYWHITE); // background
        // Draw tiles
    for (int i = 0; i < 8; i++) { // 1 to 8
        for (int j = 0; j < 8; j++) { // A to G
            DrawRectangle(
                boardPosition.x + i * tileWidth, 
                boardPosition.y + j * tileWidth,
                tileWidth, tileWidth,
                (((i + j) & 1) == whiteSideDown) ? currentBoardTheme->boardColorLight 
                                        : currentBoardTheme->boardColorDark
            );
        }
    }
    // Render Labels
    char* ranks = "12345678";
    char* files = "abcdefgh";
    char* symb = MemAlloc(sizeof(char));
    Vector2 symbolPosition;
    // ranks
    for (int i = 0; i < i; i++) {
        *symb = (char)*(ranks + i);
        symbolPosition.x = boardPosition.x - 
        DrawTextEx(boardTextFont, symb, )
    }
    MemFree(symb);
}
void UnloadGameScreen(void) 
{
    MemFree(currentBoardTheme);
    currentBoardTheme = NULL;
    MemFree(boardTextFont);
    boardTextFont = NULL;
}
int GameScreenEnded(void)
{

}