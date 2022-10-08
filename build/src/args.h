#ifndef ARGS_H
#define ARGS_H

#include "args_types.h"

#include <stddef.h>

void args_usage(const char *name, const char *description);
int args_handle(const char *name, const char *description, const arg_t *args, size_t args_size, const mode_desc_t *modes, size_t modes_size, int argc, const char *argv[], void **params);

#endif