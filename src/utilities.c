#include "utilities.h"
#define ERROR_BUFFER_SIZE 1024

int isOnButton(int x, int y, Button* button) 
{
    return ((x > button->x) && (x < button->x + button->width) 
        && (y > button->y) && (y < button->y + button->height)) ?
        1 : 0;
}

void throw_error(int line_number, const char* file_name, const char* message_format, ...)
{
    int cnt = 0;
    int i = 0;
    while (message_format[i] != '\0') {
        if (message_format[i] == '%') 
            cnt ++;
        i++;
    }

    va_list ptr;
    va_start(ptr, message_format);
    char* buffer = malloc(strlen(message_format) + ERROR_BUFFER_SIZE + cnt * 4);
    if (buffer == NULL) {printf("ERROR: Failed to allocate memory for error message\n"); exit(1);}
    printf("\nERROR: ");
    vsprintf_s(buffer, strlen(message_format) + ERROR_BUFFER_SIZE + cnt * 4, message_format, ptr);
    printf(buffer);
    printf(" (Line %d, File %s)\n", line_number, file_name);
    free(buffer);
    exit(1);
}