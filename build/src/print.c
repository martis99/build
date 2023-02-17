#include "print.h"

#include "defines.h"

int p_fprintf(FILE *f, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret;
#if defined(B_WIN)
	ret = vfprintf_s(f, fmt, args);
#else
	ret = vfprintf(f, fmt, args);
#endif
	va_end(args);
	return ret;
}

int p_vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	va_list copy;
	va_copy(copy, args);
	int ret;
#if defined(B_WIN)
	ret = vsnprintf(buf, size, fmt, copy);
#else
	ret = vsnprintf(buf, size, fmt, copy);
#endif
	va_end(copy);
	return ret;
}

int p_vsprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	va_list copy;
	va_copy(copy, args);
	int ret;
#if defined(B_WIN)
	ret = vsprintf_s(buf, size, fmt, args);
#else
	ret = vsprintf(buf, fmt, args);
#endif
	va_end(copy);
	return ret;
}
