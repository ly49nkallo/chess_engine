#include "raylib.h"
#include "stdlib.h"
#include "stdio.h"
#include "utilities.h"
#include "chess_engine.h"

int isOnButton(int x, int y, Button* button) 
{
    return ((x > button->x) && (x < button->x + button->width) 
        && (y > button->y) && (y < button->y + button->height)) ?
        1 : 0;
}


