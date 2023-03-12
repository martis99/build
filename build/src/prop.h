#ifndef PROP_H
#define PROP_H

#include "array.h"
#include "str.h"

#include <stddef.h>

typedef struct prop_str_s {
	const char *path;
	union {
		char *data;
		const char *cdata;
	};
	unsigned int len;
	unsigned int line;
	unsigned int col;
} prop_str_t;

typedef struct prop_s {
	int set;
	union {
		array_t arr;
		prop_str_t value;
	};
	unsigned int mask;
} prop_t;

typedef int (*prop_parse_fn)(prop_str_t *data, prop_t *prop);
typedef void (*prop_print_fn)(const prop_t *prop);

typedef struct prop_pol_s {
	str_t name;
	prop_print_fn print;
	unsigned int arr;
	const str_t *str_table;
	size_t str_table_len;
} prop_pol_t;

int props_parse_file(prop_str_t data, prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size);
void props_print(const prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size);

int prop_cmp(const prop_str_t *l, const prop_str_t *r);

void prop_print_arr(const prop_t *prop);
void prop_print_flags(const prop_t *prop, const str_t *str_table, size_t str_table_len);

void props_free(prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size);

int convert_slash(char *dst, unsigned int dst_len, const char *src, size_t src_len);

#endif
