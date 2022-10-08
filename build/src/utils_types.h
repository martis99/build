#ifndef UTILS_TYPES_H
#define UTILS_TYPES_H

#include <Windows.h>

typedef struct path_s {
	unsigned int len;
	char path[MAX_PATH];
} path_t;

typedef struct pathv_s {
	unsigned int len;
	const char* path;
} pathv_t;

typedef struct prop_str_s {
	const char *path;
	char *data;
	unsigned int len;
	unsigned int cur;
	unsigned int start;
	unsigned int line;
	unsigned int line_start;
} prop_str_t;

typedef struct str_s {
	const char *data;
	unsigned int len;
} str_t;

#endif