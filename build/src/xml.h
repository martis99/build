#ifndef XML_H
#define XML_H

#include "array.h"

typedef struct xml_str_s {
	union {
		const char *data;
		char *tdata;
	};
	unsigned int len;
	int mem;
} xml_str_t;

typedef struct xml_attr_s {
	xml_str_t name;
	xml_str_t val;
} xml_attr_t;

typedef struct xml_tag_s {
	xml_str_t name;
	array_t attrs;
	array_t childs;
	xml_str_t val;
} xml_tag_t;

typedef struct xml_s {
	xml_tag_t root;
} xml_t;

int xml_init(xml_t *xml);
int xml_free(xml_t *xml);

xml_attr_t *xml_add_attr(xml_tag_t *tag, const char *name, unsigned int name_len, const char *val, unsigned int val_len);
xml_attr_t *xml_add_attr_c(xml_tag_t *tag, const char *name, unsigned int name_len, const char *val, unsigned int val_len);

xml_attr_t *xml_add_attr_v(xml_tag_t *tag, const char *name, unsigned int name_len, const char *format, va_list args);
xml_attr_t *xml_add_attr_f(xml_tag_t *tag, const char *name, unsigned int name_len, const char *format, ...);

xml_tag_t *xml_add_child(xml_tag_t *tag, const char *name, unsigned int name_len);
xml_tag_t *xml_add_child_val(xml_tag_t *tag, const char *name, unsigned int name_len, const char *val, unsigned int val_len);
xml_attr_t *xml_add_child_val_v(xml_tag_t *tag, const char *name, unsigned int name_len, const char *format, va_list args);
xml_attr_t *xml_add_child_val_f(xml_tag_t *tag, const char *name, unsigned int name_len, const char *format, ...);

xml_str_t *xml_set_val(xml_tag_t *tag, const char *val, unsigned int val_len);

int xml_save(xml_t *xml, void *file);

#endif
