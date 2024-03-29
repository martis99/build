#include "prop.h"

#include "cstr.h"
#include "defines.h"
#include "mem.h"
#include "platform.h"
#include "str.h"

#if defined(C_WIN)
	#define PLATFORM "win"
#else
	#define PLATFORM "linux"
#endif

static uint val_to_mask(const prop_str_t *val, const str_t *table, size_t table_len)
{
	for (size_t i = 0; i < table_len; i++) {
		if (cstr_eq(table[i].data, table[i].len, val->data, val->len)) {
			return (uint)i;
		}
	}

	ERR_LOGICS("unknown value: '%.*s'", val->path, val->line, val->col, (int)val->len, val->data);
	return 0;
}

static int parse_value(prop_str_t *data, prop_t *prop, str_t *value, const str_t *table, size_t table_len, const str_t *line, int arr)
{
	str_t platform = { 0 };
	;
	if (!cstr_eqn(value->data, value->len, CSTR("http"), 4) && !str_chr(*value, &platform, value, ':') && !str_eqc(platform, CSTR(PLATFORM))) {
		return 1;
	}

	data->cdata = value->data;
	data->len   = value->len;
	data->col   = (uint)(value->data - line->data);

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
	arr_init(&prop->arr, 8, sizeof(prop_str_t));
	prop->flags |= PROP_ARR;
	prop->mask = 0;

	str_t value = *arr;
	str_t next  = { 0 };
	int end	    = 0;

	while (!end) {
		if (str_cstr(value, &value, &next, CSTR(", "))) {
			value.len = (size_t)(arr->data + arr->len - value.data);
			end	  = 1;
		}

		if (!parse_value(&data, prop, &value, table, table_len, line, 1)) {
			arr_app(&prop->arr, &data);
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

	if (str_cstr(*line, &name, &value, CSTR(": "))) {
		ERR_SYNTAX("missing ': '", data.path, data.line, 0);
		return 1;
	}

	if (name.len <= 0) {
		ERR_STRUCT("name missing", data.path, data.line, 0);
		return 1;
	}

	if (value.len <= 0) {
		ERR_STRUCT("value of property '%.*s' is missing", data.path, data.line, (int)(value.data - line->data), (int)name.len, name.data);
		return 1;
	}

	size_t props_pol_len = props_pol_size / sizeof(prop_pol_t);
	for (size_t i = 0; i < props_pol_len; i++) {
		const prop_pol_t *pol = &props_pol[i];
		if (str_eq(name, pol->name)) {
			if (props[i].flags & PROP_SET) {
				ERR_LOGICS("property redefinition", data.path, data.line, 0);
				return 1;
			}
			props[i].flags |= PROP_SET;

			if (props_pol[i].arr) {
				return parse_arr(data, &props[i], &value, props_pol[i].str_table, props_pol[i].str_table_len, line);
			}

			if (parse_value(&data, &props[i], &value, props_pol[i].str_table, props_pol[i].str_table_len, line, 0)) {
				props[i].flags &= ~PROP_SET;
			} else {
				props[i].value = data;
			}

			return 0;
		}
	}

	ERR_LOGICS("unknown property '%.*s'", data.path, data.line, 0, (int)name.len, name.data);
	return 1;
}

int props_parse_file(prop_str_t data, prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size)
{
	int ret = 0;

	mem_set(props, 0, props_pol_size / sizeof(prop_pol_t) * sizeof(prop_t));

	data.line = 0;

	str_t line = {
		.data = data.data,
		.len  = data.len,
	};
	str_t next = { 0 };
	int end	   = 0;

	while (!end) {
		if (str_chr(line, &line, &next, '\n')) {
			str_chr(line, &line, &next, '\0');
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
				INFP("    %.*s:", (int)props_pol[i].name.len, props_pol[i].name.data);
				if (props_pol[i].str_table) {
					prop_print_flags(&props[i], props_pol[i].str_table, props_pol[i].str_table_len);
				} else {
					prop_print_arr(&props[i]);
				}
			} else {
				if (props_pol[i].str_table) {
					INFP("    %.*s: %s", (int)props_pol[i].name.len, props_pol[i].name.data, props_pol[i].str_table[props[i].mask].data);
				} else {
					INFP("    %.*s: '%.*s'", (int)props_pol[i].name.len, props_pol[i].name.data, (int)props[i].value.len, props[i].value.data);
				}
			}
		}
	}
	INFF();
}

int prop_eq(const prop_str_t *l, const prop_str_t *r)
{
	return cstr_eq(l->data, l->len, r->data, r->len);
}

void prop_print_arr(const prop_t *prop)
{
	for (uint j = 0; j < prop->arr.cnt; j++) {
		prop_str_t *val = arr_get(&prop->arr, j);
		if (val->data == NULL) {
			INFP("%s", "        <null>");
			continue;
		}

		INFP("        '%.*s'", (int)val->len, val->data);
	}
}

void prop_print_flags(const prop_t *prop, const str_t *str_table, size_t str_table_len)
{
	for (size_t i = 0; i < str_table_len; i++) {
		if (prop->mask & (1 << i)) {
			INFP("        '%.*s'", (int)str_table[i].len, str_table[i].data);
		}
	}
}

void prop_def(prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size)
{
	size_t props_pol_len = props_pol_size / sizeof(prop_pol_t);
	for (size_t i = 0; i < props_pol_len; i++) {
		if (!(props[i].flags & PROP_SET) && props_pol[i].def.data != NULL) {
			props[i].value = (prop_str_t){
				.cdata = props_pol[i].def.data,
				.len   = props_pol[i].def.len,
			};
			props[i].flags |= PROP_SET;
		}
	}
}

void prop_free(prop_t *prop)
{
	if (prop->flags & PROP_ARR) {
		arr_free(&prop->arr);
		prop->flags &= ~PROP_ARR;
	}
}

void props_free(prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size)
{
	size_t props_pol_len = props_pol_size / sizeof(prop_pol_t);
	for (size_t i = 0; i < props_pol_len; i++) {
		prop_free(&props[i]);
	}
}

size_t convert_slash(char *dst, size_t dst_len, const char *src, size_t src_len)
{
	mem_cpy(dst, dst_len, src, src_len);
	for (size_t i = 0; i < src_len; i++) {
		if (dst[i] == '\\') {
			dst[i] = '/';
		}
	}
	return src_len;
}

size_t convert_backslash(char *dst, size_t dst_len, const char *src, size_t src_len)
{
	mem_cpy(dst, dst_len, src, src_len);
	for (size_t i = 0; i < src_len; i++) {
		if (dst[i] == '/') {
			dst[i] = '\\';
		}
	}
	return src_len;
}

size_t invert_slash(char *str, size_t str_len)
{
	for (size_t i = 0; i < str_len; i++) {
		if (str[i] == '\\') {
			str[i] = '/';
		}
	}
	return str_len;
}
