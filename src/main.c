#include "raylib.h"
#include "screens.h"
#include "utilities.h"
#include <stdio.h>

#if defined(PLATFORM_WEB)
    #error "Web is not supported"
#endif

static ScreenState currentScreenState;

static int screenWidth = 800;
static int screenHeight = 550;
static ScreenState startingScreenState = GAME;

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
    currentScreenState = startingScreenState;
    ChangeScreen(startingScreenState);
}

// Update game (one frame)
void UpdateFrame(void)
{
    switch(currentScreenState) {
        case MENU:
            title_screen_update();
            if (title_screen_ended() != 0)
                ChangeScreen(title_screen_ended());
            break;
        case GAME:
            game_screen_update();
            if (game_screen_ended() != 0)
                ChangeScreen(game_screen_ended());
            break;
        default:
            throw_error(__LINE__, __FILE__, "ERROR: Unhandled Screen Update %d\n", currentScreenState);
            break;
    }
}

// Handle Screen Transitions
void ChangeScreen(ScreenState newScreenState) 
{
    if (&currentScreenState != (void*)0) // if currentScreenState is NULL
        printf("Changing Screen from ID:%d to ID:%d\n", currentScreenState, newScreenState);

    if (currentScreenState == newScreenState)
        printf("ERROR: Changing to already running screen state ID:%d\n", currentScreenState);

    // Handle unloading previous screen
    switch(currentScreenState) {
        case MENU: title_screen_unload(); break;
        case GAME: game_screen_unload(); break;
        default: 
            if (&currentScreenState != ((void*)0)) {
                printf("ERROR: Unhandled Screen Unload %d\n", currentScreenState);
            } break;
    }

    // Handle initializing new screen
    switch(newScreenState) {
        case MENU: title_screen_init(); break;
        case GAME: game_screen_init(); break;
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
            title_screen_draw();
            break;
        case GAME:
            game_screen_draw();
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
