#include "prop.h"

#include "cstr.h"
#include "defines.h"
#include "ini_parse.h"
#include "mem.h"
#include "platform.h"
#include "str.h"

#if defined(C_WIN)
	#define PLATFORM "win"
#else
	#define PLATFORM "linux"
#endif

static size_t get_prop_pol(const prop_pol_t *props_pol, size_t props_pol_size, str_t name)
{
	size_t props_pol_len = props_pol_size / sizeof(prop_pol_t);
	for (size_t i = 0; i < props_pol_len; i++) {
		if (str_eq(name, props_pol[i].name)) {
			return i;
		}
	}
	return -1;
}

static uint val_to_mask(str_t val, const str_t *table, size_t table_len)
{
	for (size_t i = 0; i < table_len; i++) {
		if (cstr_eq(table[i].data, table[i].len, val.data, val.len)) {
			return (uint)i;
		}
	}

	return 0;
}

static void parse_value(prop_str_t *data, prop_t *prop, str_t val, const str_t *table, size_t table_len, int flags)
{
	if ((flags & PPF_DIR) && val.data[val.len - 1] != '/' && val.data[val.len - 1] != '\\') {
		data->val = strz(val.len + 2);
		str_cpyd(val, &data->val);
		((char *)data->val.data)[data->val.len++] = '/';
		((char *)data->val.data)[data->val.len]	  = '\0';
	} else {
		data->val = strz(val.len + 1);
		str_cpyd(val, &data->val);
	}

	if (table) {
		if (flags & PPF_ARR) {
			prop->mask |= 1 << val_to_mask(val, table, table_len);
		} else {
			prop->mask = val_to_mask(val, table, table_len);
		}
	}
}

static void parse_arr(ini_t *ini, ini_pair_data_t *pair, prop_t *prop, const str_t *table, size_t table_len, int flags)
{
	arr_init(&prop->arr, 8, sizeof(prop_str_t));
	prop->flags |= PROP_ARR;
	prop->mask = 0;

	str_t *val;
	ini_val_foreach(&ini->vals, pair->vals, val)
	{
		prop_str_t data = { 0 };
		parse_value(&data, prop, *val, table, table_len, flags);
		arr_app(&prop->arr, &data);
	}
}

static void parse_props(ini_t *ini, ini_sec_data_t *sec, prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size)
{
	ini_pair_data_t *pair;
	ini_pair_foreach(&ini->pairs, sec->pairs, pair)
	{
		if (pair->key.data == NULL) {
			continue;
		}

		size_t i = get_prop_pol(props_pol, props_pol_size, pair->key);
		if (i == (size_t)-1) {
			continue;
		}

		props[i].flags |= PROP_SET;

		if (props_pol[i].flags & PPF_ARR) {
			parse_arr(ini, pair, &props[i], props_pol[i].str_table, props_pol[i].str_table_len, props_pol[i].flags);
		} else {
			str_t *val;
			ini_val_foreach(&ini->vals, pair->vals, val)
			{
				parse_value(&props[i].value, &props[i], *val, props_pol[i].str_table, props_pol[i].str_table_len, props_pol[i].flags);
				break;
			}
		}
	}
}

int props_parse_file(prop_str_t data, const ini_prs_t *ini_prs, prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size)
{
	int ret = 0;

	mem_set(props, 0, props_pol_size / sizeof(prop_pol_t) * sizeof(prop_t));

	ini_t ini = { 0 };
	ini_init(&ini, 4, 16, 8);

	ini_prs_parse(ini_prs, data.val, &ini);

	ini_sec_data_t *sec;
	ini_sec_foreach(&ini.secs, sec)
	{
		if (sec->name.data == NULL || str_eq(sec->name, STR(PLATFORM))) {
			parse_props(&ini, sec, props, props_pol, props_pol_size);
		}
	}

	ini_free(&ini);

	return ret;
}

void props_print(const prop_t *props, const prop_pol_t *props_pol, size_t props_pol_size)
{
	size_t props_pol_len = props_pol_size / sizeof(prop_pol_t);
	for (size_t i = 0; i < props_pol_len; i++) {
		if (props_pol[i].print) {
			props_pol[i].print(&props[i]);
		} else {
			if (props_pol[i].flags & PPF_ARR) {
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
					INFP("    %.*s: '%.*s'", (int)props_pol[i].name.len, props_pol[i].name.data, (int)props[i].value.val.len,
					     props[i].value.val.data);
				}
			}
		}
	}
	INFF();
}

int prop_eq(const prop_str_t *l, const prop_str_t *r)
{
	return str_eq(l->val, r->val);
}

void prop_print_arr(const prop_t *prop)
{
	for (uint j = 0; j < prop->arr.cnt; j++) {
		prop_str_t *val = arr_get(&prop->arr, j);
		if (val->val.data == NULL) {
			INFP("%s", "        <null>");
			continue;
		}

		INFP("        '%.*s'", (int)val->val.len, val->val.data);
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
			props[i].value.val = strc(props_pol[i].def.data, props_pol[i].def.len);
			props[i].flags |= PROP_SET;
			props[i].ref = 1;
		}
	}
}

void prop_free(prop_t *prop)
{
	if (prop->ref || !(prop->flags & PROP_SET)) {
		return;
	}

	if (prop->flags & PROP_ARR) {
		prop_str_t *val;
		arr_foreach(&prop->arr, val)
		{
			str_free(&val->val);
		}
		arr_free(&prop->arr);
	} else {
		str_free(&prop->value.val);
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
