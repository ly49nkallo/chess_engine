#ifndef CE_UTILITIES_H
#define CE_UTILITIES_H

typedef struct Button{
    int x, y;
    int width, height;
    int isSelected;
    int isPressed;
} Button;

int isOnButton(int, int, Button*);

#endif // CE_UTILITIES_H