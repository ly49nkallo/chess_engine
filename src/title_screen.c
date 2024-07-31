#include "raylib.h"
#include "screens.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "utilities.h"

static long frameCount;
static float rotation;
static const char* titleText = "Simple Chess Engine!";
static const char* startText = "Start";
static Font titleScreenFont;
static Vector2 center, titleTextPos, startTextPos;
static float screenWidth, screenHeight;
typedef struct Button{
    float x, y;
    float width, height;
    int isSelected;
    int isPressed;
} Button;

int isOnButton(int x, int y, Button* button) 
{
    return ((x > button->x) && (x < button->x + button->width) 
        && (y > button->y) && (y < button->y + button->height)) ?
        1 : 0;
}

Button startButton;
static int screenEnded;


void title_screen_init(void) 
{
    screenEnded = 0;
    frameCount = 0;
    rotation = 0.f;
    titleScreenFont = LoadFontEx("resources/Philosopher-Bold.ttf", 40, 0, 250);
    printf("INFO: FONT: Loaded title screen font sucessfully\n");

    screenWidth = (float)GetScreenWidth();
    screenHeight = (float)GetScreenHeight();
    center.x = screenWidth/2;
    center.y = screenHeight/2;

    // Title Text
    Vector2 titleTextDims = MeasureTextEx(titleScreenFont, titleText, 40, 5); 
    float titleTextWidth = titleTextDims.x;
    float titleTextHeight = titleTextDims.y;
    titleTextPos.x = screenWidth/2 - titleTextWidth/2;
    titleTextPos.y = screenHeight/4 - titleTextHeight/2; // title is 1/4 up the screen

    // Start Text
    Vector2 startTextDims = MeasureTextEx(titleScreenFont, startText, 20, 5); 
    float startTextWidth = startTextDims.x;
    float startTextHeight = startTextDims.y;
    startTextPos.x = screenWidth/2 - startTextWidth/2;
    startTextPos.y = screenHeight/2 - startTextHeight/2; // start button is in center

    // Start Button
    startButton.width = screenWidth/4;
    startButton.height = screenHeight/4;
    startButton.x = center.x - (startButton.width/2.f);
    startButton.y = center.y - (startButton.height/2.f);
    printf("INFO: Loaded title screen sucessfully\n");
}

void title_screen_update(void) 
{
    rotation += 0.5;
    if (rotation >= 360.0f)
        rotation = 0.0f;
    // Check if mouse on over start button
    int mouseX = GetMouseX();
    int mouseY = GetMouseY();
    if (isOnButton(mouseX, mouseY, &startButton)) 
        startButton.isSelected = 1;
    else
        startButton.isSelected = 0;
    if (startButton.isSelected && IsMouseButtonReleased(0)) {
        printf("Mouse Clicked!\n");
        screenEnded = (int)GAME;
    }
    frameCount++;
}

void title_screen_draw(void) 
{
    DrawRectangle(0, 0, screenWidth, screenHeight, RAYWHITE); // background
    DrawCircleSector(center, screenHeight / 2, 0.0f + rotation, 90.0f + rotation, 20, GRAY);
    DrawCircleSector(center, screenHeight / 2, 180.0f + rotation, 270.0f + rotation, 20, GRAY);
    DrawTextEx(titleScreenFont, titleText, titleTextPos, 40, 5, BLACK);

    // Draw Button
    DrawRectangle((int)startButton.x, (int)startButton.y, (int)startButton.width, (int)startButton.height, DARKGRAY);
    Rectangle rec = {startButton.x, startButton.y, startButton.width, startButton.height};
    const Color highlightColor = WHITE;
    const Color normalColor = BLACK;
    if (startButton.isSelected)
    {
        DrawRectangleLinesEx(rec, 10.f, highlightColor); 
        DrawTextEx(titleScreenFont, startText, startTextPos, 20, 5, highlightColor);
    }
    else
    {
        DrawRectangleLinesEx(rec, 10.f, normalColor); 
        DrawTextEx(titleScreenFont, startText, startTextPos, 20, 5, normalColor);
    }
}

void title_screen_unload(void) 
{
    UnloadFont(titleScreenFont);
    printf("Unloaded Title Screen\n");
}

int title_screen_ended(void)
{
    return screenEnded;
}
