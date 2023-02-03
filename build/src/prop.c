#include "prop.h"

#include "defines.h"
#include "utils.h"

#include <string.h>

int parse_char(prop_str_t *data, char c)
{
	if (data->cur >= data->len) {
		if (c == '\n') {
			ERR_SYNTAX("unexpected end of file, expected: '\\n' (%d)", data->path, data->line + 1, data->cur - data->line_start + 1, '\n');
		} else {
			ERR_SYNTAX("unexpected end of file, expected: '%c' (%d)", data->path, data->line + 1, data->cur - data->line_start + 1, c, c);
		}
		return 1;
	}

	if (data->data[data->cur] != c) {
		if ((data->data[data->cur] >= 'A' && data->data[data->cur] <= 'Z') || (data->data[data->cur] >= 'a' && data->data[data->cur] <= 'z') ||
		    (data->data[data->cur] >= '0' && data->data[data->cur] <= '9')) {
			if (c == '\n') {
				ERR_SYNTAX("missing token: '\\n' (%d)", data->path, data->line + 1, data->cur - data->line_start + 1, '\n');
			} else {
				ERR_SYNTAX("missing token: '%c' (%d)", data->path, data->line + 1, data->cur - data->line_start + 1, c, c);
			}
		} else {
			if (c == '\n') {
				ERR_SYNTAX("unexpected token: '%c' (%d), expected: '\\n' (%d)", data->path, data->line + 1, data->cur - data->line_start + 1,
					   data->data[data->cur], data->data[data->cur], data->data[data->cur]);
			} else if (data->data[data->cur] == '\n') {
				ERR_SYNTAX("unexpected token: '\\n' (%d), expected: '%c' (%d)", data->path, data->line + 1, data->cur - data->line_start + 1,
					   data->data[data->cur], c, c);
			} else {
				ERR_SYNTAX("unexpected token: '%c' (%d), expected: '%c' (%d)", data->path, data->line + 1, data->cur - data->line_start + 1,
					   data->data[data->cur], data->data[data->cur], c, c);
			}
			data->cur++;
		}
		return 1;
	}

	data->cur++;
	return 0;
}

static int parse_prop_name(prop_str_t *data, prop_str_t *value)
{
	unsigned int start = data->cur;

	if (read_upper(data, NULL) == 0) {
		ERR_STRUCT("name missing", data->path, data->line + 1, start - data->line_start + 1);
		return 1;
	}

	*value = (prop_str_t){
		.path	    = data->path,
		.data	    = &data->data[start],
		.start	    = start,
		.len	    = data->cur - start,
		.line	    = data->line,
		.line_start = data->line_start,
	};

	return 0;
}

static int parse_str_table(prop_str_t *data, prop_t *prop, const str_t *table, size_t table_len)
{
	for (int i = 0; i < table_len; i++) {
		if (table[i].len == prop->value.len && memcmp(table[i].data, prop->value.data, prop->value.len) == 0) {
			return i;
		}
	}
	return 0;
}

static int prop_parse_arr(prop_str_t *data, prop_t *prop, prop_parse_fn parse, const str_t *table, size_t table_len)
{
	int ret = 0;

	if (table) {
		prop->mask = 0;
	} else {
		array_init(&prop->arr, 8, sizeof(prop_str_t));
	}

	while (data->cur < data->len && data->data[data->cur] != '\n') {
		prop_t element = { 0 };
		ret += parse(data, &element);
		if (ret == 0) {
			if (table) {
				prop->mask |= 1 << parse_str_table(data, &element, table, table_len);
			} else {
				array_add(&prop->arr, &element.value);
			}
		}

		if (data->data[data->cur] == ',') {
			ret += parse_char(data, ',');
			ret += parse_char(data, ' ');
		} else {
			break;
		}
	}

	return ret;
}

static int parse_prop(prop_str_t *data, prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size)
{
	int ret = 0;

	prop_str_t name = { 0 };

	unsigned int col = data->cur - data->line_start;

	int ret_name = parse_prop_name(data, &name);

	ret += parse_char(data, ':');
	ret += parse_char(data, ' ');

	if (name.data == NULL || ret_name != 0) {
		read_printable(data);
		return ret;
	}

	int found	     = 0;
	size_t props_pol_len = props_pol_size / sizeof(prop_pol_t);
	for (int i = 0; i < props_pol_len; i++) {
		if (name.len == strlen(props_pol[i].name) && memcmp(name.data, props_pol[i].name, name.len) == 0) {
			if (props[i].set) {
				ERR_LOGICS("%s already set", data->path, data->line + 1, col + 1, props_pol[i].name);
				ret++;
				return ret;
			}
			props[i].set = 1;

			if (!props_pol[i].parse) {
				return ret;
			}

			switch (props_pol[i].dim) {
			case PROP_DIM_ARRAY:
				return prop_parse_arr(data, &props[i], props_pol[i].parse, props_pol[i].str_table, props_pol[i].str_table_len);
			default: {
				int r = props_pol[i].parse(data, &props[i]);
				ret += r;
				if (r != 0) {
					return ret;
				}

				if (!props_pol[i].str_table) {
					return ret;
				}

				props[i].mask = parse_str_table(data, &props[i], props_pol[i].str_table, props_pol[i].str_table_len);
			}
			}

			return ret;
		}
	}

	ERR_LOGICS("unknown property '%.*s'", data->path, data->line + 1, col + 1, name.len, name.data);
	prop_str_t value = { 0 };
	read_printable(data);
	return ret;
}

int props_parse_file(prop_str_t *data, prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size)
{
	int ret = 0;

	int s = sizeof(prop_pol_t);
	memset(props, 0, props_pol_size / sizeof(prop_pol_t) * sizeof(prop_t));

	while (data->cur < data->len) {
		ret += parse_prop(data, props, props_pol, props_pol_size);

		if (data->cur >= data->len) {
			break;
		}

		if (data->data[data->cur] == '\n') {
			data->cur++;
			data->line++;
			data->line_start = data->cur;
		} else if (data->data[data->cur] == '\0') {
			break;
		} else {
			ERR_SYNTAX("unexpected token: '%c' (%d), expected: '\\n' (%d)", data->path, data->line + 1, data->cur - data->line_start + 1,
				   data->data[data->cur], data->data[data->cur], data->data[data->cur]);
		}
	}

	data->cur = 0;

	return ret;
}

void props_print(const prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size)
{
	size_t props_pol_len = props_pol_size / sizeof(prop_pol_t);
	for (size_t i = 0; i < props_pol_len; i++) {
		if (props_pol[i].print) {
			props_pol[i].print(&props[i]);
		} else {
			switch (props_pol[i].dim) {
			case PROP_DIM_ARRAY:
				INFP("    %s:", props_pol[i].name);
				if (props_pol[i].str_table) {
					prop_print_flags(&props[i], props_pol[i].str_table, props_pol[i].str_table_len);
				} else {
					prop_print_arr(&props[i]);
				}
				break;
			default:
				if (props_pol[i].str_table) {
					INFP("    %s: %s", props_pol[i].name, props_pol[i].str_table[props[i].mask].data);
				} else {
					INFP("    %s: '%.*s'", props_pol[i].name, props[i].value.len, props[i].value.data);
				}
				break;
			}
		}
	}
	INFF();
}

int prop_parse_word(prop_str_t *data, prop_t *prop)
{
	unsigned int start = data->cur;

	if (read_name(data) == 0) {
		ERR_STRUCT("value missing", data->path, data->line + 1, start - data->line_start + 1);
		return 1;
	}

	prop->value = (prop_str_t){
		.path	    = data->path,
		.data	    = &data->data[start],
		.start	    = start,
		.len	    = data->cur - start,
		.line	    = data->line,
		.line_start = data->line_start,
	};

	return 0;
}

int prop_parse_path(prop_str_t *data, prop_t *prop)
{
	unsigned int start = data->cur;

	if (read_path(data, NULL) == 0) {
		ERR_STRUCT("value missing", data->path, data->line + 1, start - data->line_start + 1);
		return 1;
	}

	prop->value = (prop_str_t){
		.path	    = data->path,
		.data	    = &data->data[start],
		.start	    = start,
		.len	    = data->cur - start,
		.line	    = data->line,
		.line_start = data->line_start,
	};

	return 0;
}

int prop_parse_printable(prop_str_t *data, prop_t *prop)
{
	unsigned int start = data->cur;

	if (read_printable(data, NULL) == 0) {
		ERR_STRUCT("value missing", data->path, data->line + 1, start - data->line_start + 1);
		return 1;
	}

	prop->value = (prop_str_t){
		.path	    = data->path,
		.data	    = &data->data[start],
		.start	    = start,
		.len	    = data->cur - start,
		.line	    = data->line,
		.line_start = data->line_start,
	};

	return 0;
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
		if (props_pol[i].dim == PROP_DIM_ARRAY) {
			array_free(&props[i].arr);
		}
	}
}
