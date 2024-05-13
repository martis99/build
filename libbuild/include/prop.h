#ifndef PROP_H
#define PROP_H

#include "arr.h"
#include "ini_parse.h"
#include "str.h"
#include "type.h"

typedef struct prop_str_s {
	const char *path;
	str_t val;
	uint line;
	uint col;
} prop_str_t;

typedef enum prop_flag_s {
	PROP_SET = 1 << 0,
	PROP_ARR = 1 << 1,
} prop_flag_t;

typedef struct prop_s {
	uint flags;
	union {
		arr_t arr;
		prop_str_t value;
	};
	uint mask;
	int ref;
} prop_t;

typedef int (*prop_parse_fn)(prop_str_t *data, prop_t *prop);
typedef void (*prop_print_fn)(const prop_t *prop);

typedef struct prop_pol_s {
	str_t name;
	prop_print_fn print;
	bool arr;
	const str_t *str_table;
	size_t str_table_len;
	str_t def;
} prop_pol_t;

int props_parse_file(prop_str_t data, const ini_prs_t *ini_prs, prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size);
void props_print(const prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size);

int prop_eq(const prop_str_t *l, const prop_str_t *r);

void prop_print_arr(const prop_t *prop);
void prop_print_flags(const prop_t *prop, const str_t *str_table, size_t str_table_len);

void prop_def(prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size);

void prop_free(prop_t *prop);
void props_free(prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size);

size_t convert_slash(char *dst, size_t dst_len, const char *src, size_t src_len);
size_t convert_backslash(char *dst, size_t dst_len, const char *src, size_t src_len);

size_t invert_slash(char *str, size_t str_len);

// clang-format off
#define PSTR(_str) { .val = STR(_str) }
// clang-format on
#endif
