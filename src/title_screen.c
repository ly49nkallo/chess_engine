#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "utilities.h"

static int frameCount = 0;
static float rotation = 0.f;
static const char* titleText = "Simple Chess Engine!";
static Font titleScreenFont;
static Vector2 center, titleTextPos;
static int screenWidth, screenHeight;
Button startButton;

void InitTitleScreen(void) 
{
    printf("Before: %d, %d", screenWidth, screenHeight);
    titleScreenFont = LoadFontEx("resources/Philosopher-Bold.ttf", 40, 0, 250);
    printf("INFO: FONT: Loaded title screen font sucessfully\n");
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
    printf("After: %d, %d", screenWidth, screenHeight);
    center.x = screenWidth/2;
    center.y = screenHeight/2;
    Vector2 titleTextDims = MeasureTextEx(titleScreenFont, titleText, 40, 5); 
    int titleTextWidth = titleTextDims.x;
    titleTextPos.x = screenWidth/2 - titleTextWidth/2;
    titleTextPos.y = screenHeight/2 - titleTextWidth/2;
    startButton.width = screenWidth/4;
    startButton.height = screenHeight/4;
    startButton.x = center.x - (startButton.width/2.f);
    startButton.y = center.y - (startButton.height/2.f);
    printf("INFO: Loaded title screen sucessfully\n");
}

void UpdateTitleScreen(void) 
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
    
}

void DrawTitleScreen(void) 
{
    DrawRectangle(0, 0, screenWidth, screenHeight, RAYWHITE); // background
    DrawCircleSector(center, (float) screenHeight / 2, 0.0f + rotation, 90.0f + rotation, 20, GRAY);
    DrawCircleSector(center, (float) screenHeight / 2, 180.0f + rotation, 270.0f + rotation, 20, GRAY);
    DrawTextEx(titleScreenFont, titleText, titleTextPos, 40, 5, BLACK);

    // Draw Button
    DrawRectangle(startButton.x, startButton.y, startButton.width, startButton.height, DARKGRAY);
    Rectangle rec = {startButton.x, startButton.y, startButton.width, startButton.height};
    const Color highlightColor = WHITE;
    const Color normalColor = BLACK;
    if (startButton.isSelected)
    {
        DrawRectangleLinesEx(rec, 10.f, highlightColor); 
        DrawTextEx(titleScreenFont, "Start", center, 20, 5, highlightColor);
    }
    else
    {
        DrawRectangleLinesEx(rec, 10.f, normalColor); 
        DrawTextEx(titleScreenFont, "Start", center, 20, 5, normalColor);
    }
}

void UnloadTitleScreen(void) 
{

}
