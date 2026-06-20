#include "utilities.h"

void throw_error(int line_number, const char* file_name, const char* message_format, ...)
{
    va_list ptr;
    va_start(ptr, message_format);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, message_format, ptr);
    fprintf(stderr, " (Line %d, File %s)\n", line_number, file_name);
    va_end(ptr);
    exit(1);
}

void throw_warning(int line_number, const char *file_name, const char *message_format, ...) {
    va_list ptr;
    va_start(ptr, message_format);
    fprintf(stderr, "WARNING: ");
    vfprintf(stderr, message_format, ptr);
    fprintf(stderr, " (Line %d, File %s)\n", line_number, file_name);
    va_end(ptr);
}

void throw_info(int line_number, const char *file_name, const char *message_format, ...) {
    va_list ptr;
    va_start(ptr, message_format);
    fprintf(stderr, "INFO: ");
    vfprintf(stderr, message_format, ptr);
    fprintf(stderr, " (Line %d, File %s)\n", line_number, file_name);
    va_end(ptr);
}

void throw_not_implemented_error(int line_number, const char* file_name) {
    throw_error(line_number, file_name, "Not Implemented Yet!");
}