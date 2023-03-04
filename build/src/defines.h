#ifndef DEFINES_H
#define DEFINES_H

#include <stdio.h>

extern int G_DBG;
extern int G_SUC;
extern int G_INF;
extern int G_WRN;
extern int G_ERR;
extern int G_MSG;

#define BYTE_TO_BIN_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BIN(byte)                                                                                                                      \
	(byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'), (byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'), (byte & 0x08 ? '1' : '0'), \
		(byte & 0x04 ? '1' : '0'), (byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0')

#define CL_BK "30"
#define CL_RD "31"
#define CL_GN "32"
#define CL_YL "33"
#define CL_BL "34"
#define CL_PR "35"
#define CL_CY "36"
#define CL_WH "37"

#define ERR(_format, ...)                                                                      \
	do {                                                                                   \
		if (G_ERR) {                                                                   \
			fprintf(stderr, "\033[0;" CL_RD "m" _format "\033[0m\n", __VA_ARGS__); \
			fflush(stderr);                                                        \
		}                                                                              \
	} while (0)

#define SUC(_format, ...)                                                                      \
	do {                                                                                   \
		if (G_SUC) {                                                                   \
			fprintf(stdout, "\033[0;" CL_GN "m" _format "\033[0m\n", __VA_ARGS__); \
			fflush(stdout);                                                        \
		}                                                                              \
	} while (0)

#define MSG(_format, ...)                                                                      \
	do {                                                                                   \
		if (G_MSG) {                                                                   \
			fprintf(stdout, "\033[0;" CL_YL "m" _format "\033[0m\n", __VA_ARGS__); \
			fflush(stdout);                                                        \
		}                                                                              \
	} while (0)

#define WRN(_format, ...)                                                                      \
	do {                                                                                   \
		if (G_WRN) {                                                                   \
			fprintf(stdout, "\033[0;" CL_PR "m" _format "\033[0m\n", __VA_ARGS__); \
			fflush(stdout);                                                        \
		}                                                                              \
	} while (0)

#define DBG(_format, ...)                                                                      \
	do {                                                                                   \
		if (G_DBG) {                                                                   \
			fprintf(stdout, "\033[0;" CL_CY "m" _format "\033[0m\n", __VA_ARGS__); \
			fflush(stdout);                                                        \
		}                                                                              \
	} while (0)

#define INF(_format, ...)                                                                      \
	do {                                                                                   \
		if (G_INF) {                                                                   \
			fprintf(stdout, "\033[0;" CL_WH "m" _format "\033[0m\n", __VA_ARGS__); \
			fflush(stdout);                                                        \
		}                                                                              \
	} while (0)

#define INFP(_format, ...)                                                                     \
	do {                                                                                   \
		if (G_INF) {                                                                   \
			fprintf(stdout, "\033[0;" CL_WH "m" _format "\033[0m\n", __VA_ARGS__); \
		}                                                                              \
	} while (0)

#define INFF()                          \
	do {                            \
		if (G_INF) {            \
			fflush(stdout); \
		}                       \
	} while (0)

#define ERR_INPUT(_str, ...) ERR("%s: [input error]: " _str, __VA_ARGS__)
#define ERR_OUPUT(_str, ...) ERR("%s: [ouput error]: " _str, __VA_ARGS__)

#define ERR_STRUCT(_str, ...) ERR("%s(%d,%2d): [struct error]: " _str, __VA_ARGS__)
#define ERR_SYNTAX(_str, ...) ERR("%s(%d,%2d): [syntax error]: " _str, __VA_ARGS__)
#define ERR_LOGICS(_str, ...) ERR("%s(%d,%2d): [logics error]: " _str, __VA_ARGS__)

#define MAX(a, b) a > b ? a : b
#define MIN(a, b) a < b ? a : b

#define DATA_LEN 1024

#endif
