#include "prop.h"

#include "cstr.h"
#include "defines.h"
#include "mem.h"
#include "platform.h"
#include "str.h"

#if defined(P_WIN)
	#define PLATFORM "win"
#else
	#define PLATFORM "linux"
#endif

static int val_to_mask(const prop_str_t *val, const str_t *table, size_t table_len)
{
	for (int i = 0; i < table_len; i++) {
		if (cstr_cmp(table[i].data, table[i].len, val->data, val->len)) {
			return i;
		}
	}

	ERR_LOGICS("unknown value: '%.*s'", val->path, val->line, val->col, val->len, val->data);
	return 0;
}

static int parse_value(prop_str_t *data, prop_t *prop, str_t *value, const str_t *table, size_t table_len, const str_t *line, int arr)
{
	str_t platform = { 0 };
	;
	if (!str_chr(value, &platform, value, ':') && !str_eq_cstr(&platform, CSTR(PLATFORM))) {
		return 1;
	}

	data->cdata = value->data;
	data->len   = value->len;
	data->col   = (unsigned int)(value->data - line->data);

	if (table) {
		if (arr) {
			prop->mask |= 1 << val_to_mask(data, table, table_len);
		} else {
			prop->mask = val_to_mask(data, table, table_len);
		}
	}

	return 0;
}

static int parse_arr(prop_str_t data, prop_t *prop, const str_t *arr, const str_t *table, size_t table_len, const str_t *line)
{
	array_init(&prop->arr, 4, sizeof(prop_str_t));
	prop->mask = 0;

	str_t value = *arr;
	str_t next  = { 0 };
	int end	    = 0;

	while (!end) {
		if (str_cstr(&value, &value, &next, ", ", 2)) {
			value.len = (unsigned int)(arr->data + arr->len - value.data);
			end	  = 1;
		}

		if (!parse_value(&data, prop, &value, table, table_len, line, 1)) {
			array_add(&prop->arr, &data);
		}

		value = next;
	}

	return 0;
}

static int parse_prop(prop_str_t data, prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size, const str_t *line)
{
	if (line->len == 0) {
		return 0;
	}

	str_t name  = { 0 };
	str_t value = { 0 };

	if (str_cstr(line, &name, &value, CSTR(": "))) {
		ERR_SYNTAX("missing ': '", data.path, data.line, 0);
		return 1;
	}

	if (name.len <= 0) {
		ERR_STRUCT("name missing", data.path, data.line, 0);
		return 1;
	}

	if (value.len <= 0) {
		ERR_STRUCT("value of property '%.*s' is missing", data.path, data.line, (unsigned int)(value.data - line->data), name.len, name.data);
		return 1;
	}

	size_t props_pol_len = props_pol_size / sizeof(prop_pol_t);
	for (int i = 0; i < props_pol_len; i++) {
		const prop_pol_t *pol = &props_pol[i];
		if (str_eq_str(&name, &pol->name)) {
			if (props[i].set) {
				ERR_LOGICS("property redefinition", data.path, data.line, 0);
				return 1;
			}
			props[i].set = 1;

			if (props_pol[i].arr) {
				return parse_arr(data, &props[i], &value, props_pol[i].str_table, props_pol[i].str_table_len, line);
			}

			if (parse_value(&data, &props[i], &value, props_pol[i].str_table, props_pol[i].str_table_len, line, 0)) {
				props[i].set = 0;
			} else {
				props[i].value = data;
			}

			return 0;
		}
	}

	ERR_LOGICS("unknown property '%.*s'", data.path, data.line, 0, name.len, name.data);
	return 1;
}

int props_parse_file(prop_str_t data, prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size)
{
	int ret = 0;

	m_memset(props, 0, props_pol_size / sizeof(prop_pol_t) * sizeof(prop_t));

	data.line = 0;

	str_t line = {
		.data = data.data,
		.len  = data.len,
	};
	str_t next = { 0 };
	int end	   = 0;

	while (!end) {
		if (str_chr(&line, &line, &next, '\n')) {
			str_chr(&line, &line, &next, '\0');
			end = 1;
		}

		ret += parse_prop(data, props, props_pol, props_pol_size, &line);

		line = next;
		data.line++;
	}

	return ret;
}

void props_print(const prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size)
{
	size_t props_pol_len = props_pol_size / sizeof(prop_pol_t);
	for (size_t i = 0; i < props_pol_len; i++) {
		if (props_pol[i].print) {
			props_pol[i].print(&props[i]);
		} else {
			if (props_pol[i].arr) {
				INFP("    %.*s:", props_pol[i].name.len, props_pol[i].name.data);
				if (props_pol[i].str_table) {
					prop_print_flags(&props[i], props_pol[i].str_table, props_pol[i].str_table_len);
				} else {
					prop_print_arr(&props[i]);
				}
			} else {
				if (props_pol[i].str_table) {
					INFP("    %.*s: %s", props_pol[i].name.len, props_pol[i].name.data, props_pol[i].str_table[props[i].mask].data);
				} else {
					INFP("    %.*s: '%.*s'", props_pol[i].name.len, props_pol[i].name.data, props[i].value.len, props[i].value.data);
				}
			}
		}
	}
	INFF();
}

int prop_cmp(const prop_str_t *l, const prop_str_t *r)
{
	return cstr_cmp(l->data, l->len, r->data, r->len);
}

void prop_print_arr(const prop_t *prop)
{
	for (int j = 0; j < prop->arr.count; j++) {
		prop_str_t *val = array_get(&prop->arr, j);
		INFP("        '%.*s'", val->len, val->data);
	}
}

void prop_print_flags(const prop_t *prop, const str_t *str_table, size_t str_table_len)
{
	for (int i = 0; i < str_table_len; i++) {
		if (prop->mask & (1 << i)) {
			INFP("        '%.*s'", str_table[i].len, str_table[i].data);
		}
	}
}

void props_free(prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size)
{
	size_t props_pol_len = props_pol_size / sizeof(prop_pol_t);
	for (size_t i = 0; i < props_pol_len; i++) {
		if (props_pol[i].arr) {
			array_free(&props[i].arr);
		}
	}
}

void convert_slash(char *dst, unsigned int dst_len, const char *src, size_t len)
{
	m_cpy(dst, dst_len, src, len);
	for (int i = 0; i < len; i++) {
		if (dst[i] == '\\') {
			dst[i] = '/';
		}
	}
}
