#include "utilities.h"

void throw_error(int line_number, const char* file_name, const char* message_format, ...)
{
    va_list ptr;
    va_start(ptr, message_format);
    //char* buffer = malloc(strlen(message_format) + ERROR_BUFFER_SIZE);
    //if (buffer == NULL) {printf("ERROR: Failed to allocate memory for error message\n"); exit(1);}
    printf("ERROR: ");
    vprintf(message_format, ptr);
    // printf(buffer);
    printf(" (Line %d, File %s)\n", line_number, file_name);
    //free(buffer);
    va_end(ptr);
    exit(1);
}

void throw_warning(int line_number, const char *file_name, const char *message_format, ...) {
    va_list ptr;
    va_start(ptr, message_format);
    printf("WARNING: ");
    vprintf(message_format, ptr);
    printf(" (Line %d, File %s)\n", line_number, file_name);
    va_end(ptr);
}

void throw_info(int line_number, const char *file_name, const char *message_format, ...) {
    va_list ptr;
    va_start(ptr, message_format);
    printf("INFO: ");
    vprintf(message_format, ptr);
    printf(" (Line %d, File %s)\n", line_number, file_name);
    va_end(ptr);
}

void throw_not_implemented_error(int line_number, const char* file_name) {
    throw_error(line_number, file_name, "Not Implemented Yet!");
}