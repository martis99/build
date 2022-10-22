#ifndef ENUMS_H
#define ENUMS_H

#include "array.h"
#include "utils_types.h"

typedef struct prop_s {
	int set;
	union {
		prop_str_t value;
		unsigned int mask;
		void *ptr;
		array_t arr;
	};
} prop_t;

#endif
