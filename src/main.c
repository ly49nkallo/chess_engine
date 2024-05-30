#include "raylib.h"
#include "screens.h"
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

static void InitApp(void);         // Initialize App
static void UpdateFrame(void);       // Update Frame (one frame)
static void DrawFrame(void);         // Draw Frame (one frame)
static void UnloadFrame(void);       // Unload Frame
static void UpdateDrawFrame(void);  // Update and Draw (one frame)
static void ChangeScreen(ScreenState);     // Handle screen transition (unloading, initalizing)

static char* WindowName = "Simple Chess Engine - Alpha 1.0";

int main(void)
{
    InitWindow(screenWidth, screenHeight, WindowName);

    InitApp();

    SetTargetFPS(30);
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        UpdateDrawFrame();
    }
    // De-Initialization
    UnloadFrame();         // Unload loaded data (textures, sounds, models...)
    CloseWindow();        // Close window and OpenGL context
    return 0;
}

// Initialize game variables
void InitApp(void)
{
    currentScreenState = MENU;
    ChangeScreen(MENU);
}

// Update game (one frame)
void UpdateFrame(void)
{
    switch(currentScreenState) {
        case MENU:
            UpdateTitleScreen();
            break;
        case GAME:
            UpdateGameScreen();
            break;
        default:
            printf("ERROR: Unhandled Screen Update %d\n", currentScreenState);
            break;
    }
}

// Handle Screen Transitions
void ChangeScreen(ScreenState newScreenState) 
{
    // Handle unloading previous screen
    switch(currentScreenState) {
        case MENU: UnloadTitleScreen(); break;
        case GAME: break;
        default: 
            if (&currentScreenState != ((void *)0)) {
                printf("ERROR: Unhandled Screen Unload %d\n", currentScreenState);
            } break;
    }

    // Handle initializing new screen
    switch(newScreenState) {
        case MENU: InitTitleScreen(); break;
        case GAME: InitGameScreen(); break;
        default: printf("ERROR: Unhandled Screen State %d\n", newScreenState); break;
    }

    currentScreenState = newScreenState;
}

// Draw game (one frame)
void DrawFrame(void)
{
    BeginDrawing();
    ClearBackground(VIOLET);
    switch(currentScreenState) {
        case MENU:
            DrawTitleScreen();
            break;
        case GAME:
            //TODO
            break;
        default: printf("ERROR: Unhandled Screen State %d\n", currentScreenState); break;
    }
    EndDrawing();
}

// Unload game variables
void UnloadFrame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateFrame();
    DrawFrame();
}
