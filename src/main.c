#include "raylib.h"
#include "screens.h"
#include "utilities.h"
#include "chess_engine.h"
#include <stdio.h>
#include <string.h>

#if defined(PLATFORM_WEB)
#error "Web is not supported"
#endif


static int screen_width = 800;
static int screen_height = 600;
static int frame_rate = 60;
static const ScreenState startingScreenState = GAME;
static ScreenState currentScreenState = 0;

static void InitApp(void);             // Initialize App
static void UpdateFrame(void);         // Update Frame (one frame)
static void DrawFrame(void);           // Draw Frame (one frame)
static void UnloadFrame(void);         // Unload Frame
static void UpdateDrawFrame(void);     // Update and Draw (one frame)
static void ChangeScreen(ScreenState); // Handle screen transition (unloading, initalizing)

static const char *WindowName = "Simple Chess Engine - Alpha 1.0";

// Run the engine as a UCI client over stdin/stdout instead of the GUI.
// Triggered by passing "uci" (or "--uci") as the first argument.
static int run_uci(void)
{
    ChessBoard board;
    chess_board_init(&board);
    generate_board_from_FEN(&board, CE_FEN_STARTING_POSITION);
    uci_loop(&board);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc > 1 && (strcmp(argv[1], "uci") == 0 || strcmp(argv[1], "--uci") == 0))
        return run_uci();

    InitWindow(screen_width, screen_height, WindowName);

    InitApp();

    SetTargetFPS(frame_rate);
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update and Draw
        UpdateDrawFrame();
    }
    // De-Initialization
    UnloadFrame(); // Unload loaded data (textures, sounds, models...)
    CloseWindow(); // Close window and OpenGL context
    return 0;
}

// Initialize game variables
static void InitApp(void)
{
    ChangeScreen(startingScreenState);
    currentScreenState = startingScreenState;
}

// Update game (one frame)
static void UpdateFrame(void)
{
    switch (currentScreenState)
    {
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
static void ChangeScreen(ScreenState newScreenState)
{
    if (currentScreenState == newScreenState)
        throw_error(__LINE__, __FILE__, "ERROR: Changing to already running screen state ID:%d\n", currentScreenState);

    if (&currentScreenState != (void *)0) // if currentScreenState is NULL
        printf("INFO: Changing Screen from ID:%d to ID:%d\n", currentScreenState, newScreenState);

    // Handle unloading previous screen
    switch (currentScreenState)
    {
    case MENU:
        title_screen_unload();
        break;
    case GAME:
        game_screen_unload();
        break;
    default:
        if (&currentScreenState != ((void *)0))
        {
            printf("ERROR: Unhandled Screen Unload %d\n", currentScreenState);
        }
        break;
    }

    // Handle initializing new screen
    switch (newScreenState)
    {
    case MENU:
        title_screen_init();
        break;
    case GAME:
        game_screen_init();
        break;
    default:
        printf("ERROR: Unhandled Screen State %d\n", newScreenState);
        break;
    }

    currentScreenState = newScreenState;
}

// Draw game (one frame)
static void DrawFrame(void)
{
    BeginDrawing();
    ClearBackground(VIOLET);
    switch (currentScreenState)
    {
    case MENU:
        title_screen_draw();
        break;
    case GAME:
        game_screen_draw();
        break;
    default:
        printf("ERROR: Unhandled Screen State %d\n", currentScreenState);
        break;
    }
    EndDrawing();
}

// Unload game variables
static void UnloadFrame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
static void UpdateDrawFrame(void)
{
    UpdateFrame();
    DrawFrame();
}
