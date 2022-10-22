#ifndef ARGS_TYPES_H
#define ARGS_TYPES_H

typedef enum param_e {
	PARAM_NONE,
	PARAM_INT,
	PARAM_STR,
	PARAM_MODE,
	PARAM_SWITCH,
} param_t;

typedef struct mode_s {
	char c;
	const char *desc;
} mode_t;

typedef struct mode_desc_s {
	const char *name;
	const mode_t *modes;
	size_t len;
} mode_desc_t;

typedef int (*param_handler_fn)(const void *param, void *ret);
typedef struct arg_s {
	char c;
	const char *l;
	param_t param;
	const char *name;
	const char *desc;
	param_handler_fn handler;
} arg_t;

#endif
