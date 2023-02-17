#ifndef PRINT_H
#define PRINT_H

#include <stdarg.h>
#include <stdio.h>

int p_fprintf(FILE *f, const char *fmt, ...);

int p_vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int p_vsprintf(char *buf, size_t size, const char *fmt, va_list args);

#endif
