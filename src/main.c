#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#if defined(PLATFORM_WEB)
    #error Web is not supported
#endif

typedef enum ScreenState {MENU=0, GAME} ScreenState;
static ScreenState currentScreenState;

static int screenWidth = 800;
static int screenHeight = 450;

static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)
static char* WindowName = "Simple Chess Engine - Alpha 1.0";
static float rotation = 0.f;
static const char* titleText = "Simple Chess Engine!";
static Font titleScreenFont;

int main(void)
{
    InitWindow(screenWidth, screenHeight, WindowName);

    InitGame();

    SetTargetFPS(30);
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        UpdateDrawFrame();
    }
    // De-Initialization
    UnloadGame();         // Unload loaded data (textures, sounds, models...)
    CloseWindow();        // Close window and OpenGL context
    return 0;
}

// Initialize game variables
void InitGame(void)
{
    currentScreenState = MENU;
    titleScreenFont = LoadFontEx("resources/Philosopher-Bold.ttf", 40, 0, 250);
}

// Update game (one frame)
void UpdateGame(void)\
{
}

// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    switch(currentScreenState) {
        case MENU: {
            DrawRectangle(0, 0, screenWidth, screenHeight, GREEN); // background
            {
            Vector2 bottom_center = {screenWidth/2, screenHeight/2};
            DrawCircleSector(bottom_center, (float) screenHeight / 2, 0. + rotation, 90. + rotation, 20, RED);
            }
            Vector2 text_dims = MeasureTextEx(titleScreenFont, titleText, 40, 5); 
            int text_width = text_dims.x;
            Vector2 TextPos = {screenWidth/2 - text_width/2, screenHeight/2 - text_width/2};
            DrawTextEx(titleScreenFont, titleText, TextPos, 40, 5, BLACK);
        }
        case GAME: {

        }
    }
    EndDrawing();
    rotation += 0.5;
}

// Unload game variables
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}
