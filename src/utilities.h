#ifndef CE_UTILITIES_H
#define CE_UTILITIES_H

#include "stdlib.h"
#include "stdio.h"
#include "stdarg.h"
#include "chess_engine.h"
typedef struct Button{
    int x, y;
    int width, height;
    int isSelected;
    int isPressed;
} Button;

int isOnButton(int, int, Button*);

void throw_error(int line_number, const char* file_name, const char* message, ...);

#endif // CE_UTILITIES_H