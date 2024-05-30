#include "utilities.h"

int isOnButton(int x, int y, Button* button) 
{
    if ((x > button->x) && (x < button->x + button->width) 
        && (y > button->y) && (y < button->y + button->height))
    {
        return 1;
    }
    else
        return 0;
}