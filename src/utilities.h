#ifndef CE_UTILITIES_H
#define CE_UTILITIES_H

#include "stdlib.h"
#include "stdio.h"
#include "stdarg.h"
#include "chess_engine.h"

void throw_error(int line_number, const char* file_name, const char* message_format, ...);
void throw_warning(int line_number, const char *file_name, const char *message_format, ...);
void throw_info(int line_number, const char *file_name, const char *message_format, ...);
void throw_not_implemented_error(int line_number, const char* file_name);

#define ERROR(m, ...) throw_error(__LINE__, __FILE__, m, __VA_ARGS__)
#define WARNING(m, ...) throw_warning(__LINE__, __FILE__, m, __VA_ARGS__)
#define INFO(m, ...) throw_info(__LINE__, __FILE__, m, __VA_ARGS__)
#define NOT_IMPLEMENTED() throw_not_implemented_error(__LINE__, __FILE__)

#endif // CE_UTILITIES_H