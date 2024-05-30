#ifndef UTILITIES_H
#define UTILITIES_H
#endif

typedef struct Button{
    int x, y;
    int width, height;
    int isSelected;
    int isPressed;
} Button;

int isOnButton(int, int, Button*);