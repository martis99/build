#ifndef PROP_H
#define PROP_H

#include "types.h"

#include <stddef.h>

typedef enum prop_dim_e {
	PROP_DIM_UNKNOWN,
	PROP_DIM_SINGLE,
	PROP_DIM_ARRAY,
	__PROP_DIM_MAX,
} prop_dim_t;

typedef int (*prop_parse_fn)(prop_str_t *data, prop_t *prop);
typedef void (*prop_print_fn)(const prop_t *prop);

typedef struct prop_pol_s {
	const char *name;
	prop_parse_fn parse;
	prop_print_fn print;
	prop_dim_t dim;
	const str_t *str_table;
	size_t str_table_len;
} prop_pol_t;

int props_parse_file(prop_str_t *data, prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size);
void props_print(const prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size);

int parse_char(prop_str_t *data, char c);
int prop_parse_word(prop_str_t *data, prop_t *prop);
int prop_parse_path(prop_str_t *data, prop_t *prop);

void prop_print_arr(const prop_t *prop);
void prop_print_flags(const prop_t *prop, const str_t *str_table, size_t str_table_len);

void props_free(prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size);

#endif
