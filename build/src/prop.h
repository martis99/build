#ifndef PROP_H
#define PROP_H

#include "array.h"
#include "str.h"

#include <stddef.h>

typedef enum prop_dim_e {
	PROP_DIM_UNKNOWN,
	PROP_DIM_SINGLE,
	PROP_DIM_ARRAY,
	__PROP_DIM_MAX,
} prop_dim_t;

typedef struct prop_str_s {
	const char *path;
	char *data;
	unsigned int len;
	unsigned int cur;
	unsigned int start;
	unsigned int line;
	unsigned int line_start;
} prop_str_t;

typedef struct prop_s {
	int set;
	union {
		prop_str_t value;
		unsigned int mask;
		void *ptr;
		array_t arr;
	};
} prop_t;

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
int prop_parse_printable(prop_str_t *data, prop_t *prop);

int prop_cmp(const prop_str_t *l, const prop_str_t *r);

void prop_print_arr(const prop_t *prop);
void prop_print_flags(const prop_t *prop, const str_t *str_table, size_t str_table_len);

void props_free(prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size);

int read_char(prop_str_t *str, char c);
int read_name(prop_str_t *str);
int read_path(prop_str_t *str, prop_str_t *dst);
int read_upper(prop_str_t *str, prop_str_t *dst);
int read_printable(prop_str_t *str);

void convert_slash(char *dst, unsigned int dst_len, const char *src, size_t len);

#endif
