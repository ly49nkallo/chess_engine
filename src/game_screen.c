#include "raylib.h"
#include "screens.h"

#include <stdlib.h>
#include <stdio.h>

static long frameCount;
static int screenEnded;
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
    screenEnded = 0;
    frameCount = 0L;
    // setup default board theme
    currentBoardTheme=MemAlloc(sizeof(struct BoardTheme));
    currentBoardTheme->backgroundColor = RAYWHITE;
    currentBoardTheme->boardColorDark = GRAY;
    currentBoardTheme->boardColorLight = LIGHTGRAY;
    currentBoardTheme->pieceFontName = "default";

    whiteSideDown = 1; //Start with white side down

    boardHeight = 7 * GetScreenHeight() / 10; // board will be 70% of the height of the screen
    boardWidth = boardHeight ; // Board is a square
    boardPosition.x = GetScreenWidth() / 2 - boardWidth / 2; // Centred Board Position
    boardPosition.y = GetScreenHeight() / 2 - boardHeight / 2 - 50;

    boardTextFont = LoadFontEx("resources/Philosopher-Bold.ttf", 20, 0, 250); // for the annotations on the board
    printf("Loaded Game Screen Successfully\n");
}

void UpdateGameScreen(void) 
{
    frameCount++;
}

void DrawGameScreen(void) 
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int tileWidth = boardWidth / 8;
    DrawRectangle(0, 0, screenWidth, screenHeight, RAYWHITE); // background
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
    char* symb = MemAlloc(sizeof(char) * 2);
    if (symb == (void *)0)
    {
        printf("ERROR: Could not allocate memory");
        exit(1);
    }
    Vector2 symbolPosition;
    Vector2 center = (Vector2) {screenWidth / 2, screenHeight / 2};
    // ranks
    for (int i = 0; i < 8; i++) {
        *symb = (char)*(ranks + i);
        *(symb + 1) = '\n'; // terminate string
        Vector2 symbDims = MeasureTextEx(boardTextFont, symb, 20, 5); 
        symbolPosition.x = boardPosition.x - tileWidth/2 - symbDims.x/2;
        symbolPosition.y = boardPosition.y + tileWidth/2 - symbDims.y/2 + (tileWidth * i);
        // printf("DEBUG: Board Position: (%f, %f)", symbolPosition.x, symbolPosition.y);
        if ((symbolPosition.x <= 0 || symbolPosition.x >= screenWidth)
            || (symbolPosition.y <= 0 || symbolPosition.y >= screenHeight))
        {
            printf("ERROR: Symbol Position is out of bounds POS:(%f, %f)\n", symbolPosition.x, symbolPosition.y);
            exit(1);
        }
        DrawTextEx(boardTextFont, symb, symbolPosition, 20, 5, BLACK);
    }
    // files
    for (int i = 0; i < 8; i++) {
        *symb = (char)*(files + i);
        *(symb + 1) = '\n'; // terminate string
        Vector2 symbDims = MeasureTextEx(boardTextFont, symb, 20, 5); 
        symbolPosition.x = boardPosition.x - tileWidth/2 - symbDims.x/2 + (tileWidth * i) + tileWidth;
        symbolPosition.y = boardPosition.y + boardHeight + tileWidth/2 - symbDims.y/2 ;
        // printf("DEBUG: Board Position: (%f, %f)", symbolPosition.x, symbolPosition.y);
        if ((symbolPosition.x <= 0 || symbolPosition.x >= screenWidth)
            || (symbolPosition.y <= 0 || symbolPosition.y >= screenHeight))
        {
            printf("ERROR: Symbol Position is out of bounds POS:(%f, %f)\n", symbolPosition.x, symbolPosition.y);
            exit(1);
        }
        DrawTextEx(boardTextFont, symb, symbolPosition, 20, 5, BLACK);
    }

    MemFree(symb);
    return;
}
void UnloadGameScreen(void) 
{
    UnloadFont(boardTextFont);
}
int GameScreenEnded(void)
{
    return screenEnded;
}