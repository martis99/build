#include "xml.h"

#include "array.h"
#include "mem.h"

#include <stdio.h>
#include <stdarg.h>
#include <memory.h>

static inline int xml_init_tag(xml_tag_t *tag) {
	array_init(&tag->attrs, 2, sizeof(xml_attr_t));
	array_init(&tag->childs, 2, sizeof(xml_tag_t));
	return 0;
}

int xml_init(xml_t *xml) {
	return xml_init_tag(&xml->root);
}

static int xml_str_free(xml_str_t *str) {
	if (str->mem) {
		m_free(str->tdata, (size_t)str->len + 1);
	}
	return 0;
}

static int xml_attr_free(xml_attr_t *attr) {
	xml_str_free(&attr->name);
	xml_str_free(&attr->val);
	return 0;
}

static int xml_tag_free(xml_tag_t *tag) {
	xml_str_free(&tag->name);
	xml_str_free(&tag->val);

	for (int i = 0; i < tag->attrs.count; i++) {
		xml_attr_free(array_get(&tag->attrs, i));
	}

	for (int i = 0; i < tag->childs.count; i++) {
		xml_tag_free(array_get(&tag->childs, i));
	}

	array_free(&tag->attrs);
	array_free(&tag->childs);
	return 0;
}

int xml_free(xml_t *xml) {
	return xml_tag_free(&xml->root);
}

xml_attr_t *xml_add_attr(xml_tag_t *tag, const char *name, unsigned int name_len, const char *val, unsigned int val_len) {
	xml_attr_t attr = {
		.name = {.data = name, .len = name_len, .mem = 0},
		.val = {.data = val, .len = val_len},
	};
	return array_add(&tag->attrs, &attr);
}

xml_attr_t *xml_add_attr_c(xml_tag_t *tag, const char *name, unsigned int name_len, const char *val, unsigned int val_len) {
	xml_attr_t attr = {
		.name = {.data = name, .len = name_len, .mem = 0},
		.val = {.data = NULL, .len = val_len, .mem = 1},
	};
	attr.val.data = m_calloc((size_t)attr.val.len + 1, sizeof(char));
	memcpy(attr.val.tdata, val, attr.val.len);
	return array_add(&tag->attrs, &attr);
}

xml_attr_t *xml_add_attr_v(xml_tag_t *tag, const char *name, unsigned int name_len, const char *format, va_list args) {
	xml_attr_t attr = {
		.name = {.data = name, .len = name_len, .mem = 0},
		.val = {.data = NULL, .len = 0, .mem = 1 },
	};

	attr.val.len = vsnprintf(NULL, 0, format, args);
	attr.val.data = m_calloc((size_t)attr.val.len + 1, sizeof(char));
	vsprintf_s(attr.val.tdata, (size_t)attr.val.len + 1, format, args);

	return array_add(&tag->attrs, &attr);
}

xml_attr_t *xml_add_attr_f(xml_tag_t *tag, const char *name, unsigned int name_len, const char *format, ...) {
	va_list args;
	va_start(args, format);
	xml_attr_t *attr = xml_add_attr_v(tag, name, name_len, format, args);
	va_end(args);
	return attr;
}

xml_tag_t *xml_add_child(xml_tag_t *tag, const char *name, unsigned int name_len) {
	xml_tag_t child = {
		.name = {.data = name, .len = name_len, .mem = 0},
		.attrs = {0},
		.childs = {0},
		.val = {0},
	};
	xml_init_tag(&child);
	return array_add(&tag->childs, &child);
}

xml_tag_t *xml_add_child_val(xml_tag_t *tag, const char *name, unsigned int name_len, const char *val, unsigned int val_len) {
	xml_tag_t child = {
	.name = {.data = name, .len = name_len, .mem = 0},
	.attrs = {0},
	.childs = {0},
	.val = { .data = val, .len = val_len, .mem = 0 },
	};
	xml_init_tag(&child);
	return array_add(&tag->childs, &child);
}

xml_attr_t *xml_add_child_val_v(xml_tag_t *tag, const char *name, unsigned int name_len, const char *format, va_list args) {
	xml_tag_t child = {
	.name = {.data = name, .len = name_len, .mem = 0},
	.attrs = {0},
	.childs = {0},
	.val = {.data = NULL, .len = 0, .mem = 1 },
	};

	child.val.len = vsnprintf(NULL, 0, format, args);
	child.val.data = m_calloc((size_t)child.val.len + 1, sizeof(char));
	vsprintf_s(child.val.tdata, (size_t)child.val.len + 1, format, args);

	return array_add(&tag->childs, &child);
}

xml_attr_t *xml_add_child_val_f(xml_tag_t *tag, const char *name, unsigned int name_len, const char *format, ...) {
	va_list args;
	va_start(args, format);
	xml_attr_t *attr = xml_add_child_val_v(tag, name, name_len, format, args);
	va_end(args);
	return attr;
}

xml_str_t *xml_set_val(xml_tag_t *tag, const char *val, unsigned int val_len) {
	tag->val = (xml_str_t){
		.data = val,
		.len = val_len,
		.mem = 0,
	};
	return &tag->val;
}

static inline int xml_attr_save(const xml_attr_t *attr, void *file) {
	fprintf_s(file, " %.*s=\"%.*s\"", attr->name.len, attr->name.data, attr->val.len, attr->val.data);
	return 0;
}

static int xml_tag_save(const xml_tag_t *tag, void *file, unsigned int depth) {
	fprintf_s(file, "%*s<%.*s", depth * 2, "", tag->name.len, tag->name.data);
	for (int i = 0; i < tag->attrs.count; i++) {
		xml_attr_save(array_get(&tag->attrs, i), file);
	}

	if (tag->childs.count > 0) {
		fprintf_s(file, ">\n");

		for (int i = 0; i < tag->childs.count; i++) {
			xml_tag_save(array_get(&tag->childs, i), file, depth + 1);
		}

		fprintf_s(file, "%*s</%.*s>\n", depth * 2, "", tag->name.len, tag->name.data);
		return 0;
	} else if (tag->val.data) {
		if (tag->val.len > 0 && tag->val.data[tag->val.len - 1] == '\n') {
			fprintf_s(file, ">%.*s%*s</%.*s>\n", tag->val.len, tag->val.data, depth * 2, "", tag->name.len, tag->name.data);
		} else {
			fprintf_s(file, ">%.*s</%.*s>\n", tag->val.len, tag->val.data, tag->name.len, tag->name.data);
		}
	} else {
		fprintf_s(file, " />\n");
	}

	return 0;
}

int xml_save(xml_t *xml, void *file) {
	fprintf(file, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	for (int i = 0; i < xml->root.childs.count; i++) {
		xml_tag_save(array_get(&xml->root.childs, i), file, 0);
	}
	return 0;
}