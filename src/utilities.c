#include "utilities.h"

int isOnButton(int x, int y, Button* button) 
{
    return ((x > button->x) && (x < button->x + button->width) 
        && (y > button->y) && (y < button->y + button->height)) ?
        1 : 0;
}


PrintBoardInTerminal_FEN(const char* FEN_string)
{
    
}