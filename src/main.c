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

int main(void)
{
    InitWindow(screenWidth, screenHeight, "classic game: arkanoid");

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
            DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
            {
            Vector2 bottom_center = {(float) screenWidth / 2, (float) screenHeight};
            DrawCircleSector(bottom_center, (float) screenHeight, PI/2, PI/3, 1, RED);
            }
        }
        case GAME: {

        }
    }
    EndDrawing();
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