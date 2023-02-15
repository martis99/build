#include "mk_proj.h"

#include "defines.h"
#include "utils.h"

int mk_proj_gen(const proj_t *proj, const hashmap_t *projects, const path_t *path)
{
	const prop_str_t *name = &proj->props[PROJ_PROP_NAME].value;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	int ret = 0;

	path_t cmake_path = *path;
	if (path_child(&cmake_path, proj->rel_path.path, proj->rel_path.len)) {
		return 1;
	}

	if (!folder_exists(cmake_path.path)) {
		folder_create(cmake_path.path);
	}

	if (path_child(&cmake_path, "Makefile", 8)) {
		return 1;
	}

	FILE *fp = file_open(cmake_path.path, "w", 1);
	if (fp == NULL) {
		return 1;
	}

	MSG("generating project: %s", cmake_path.path);

	switch (type) {
	case PROJ_TYPE_EXE:
		fprintf_s(fp, "BIN:=%.*s\n", name->len, name->data);
		break;
	default:
		fprintf_s(fp, "BIN:=%.*s.a\n", name->len, name->data);
		break;
	}

	fprintf_s(fp, "SRC:=$(shell find -name '*.c')\n"
		      "OBJS:=$(SRC:.c=.o)\n");

	if (proj->all_depends.count > 0) {
		fprintf_s(fp, "DEPENDS =");

		for (int i = 0; i < proj->all_depends.count; i++) {
			prop_str_t **depend = array_get(&proj->all_depends, i);
			proj_t *dproj	    = NULL;
			if (hashmap_get(projects, (*depend)->data, (*depend)->len, &dproj)) {
				ERR("project doesn't exists: '%.*s'", (*depend)->len, (*depend)->data);
				continue;
			}
			char buf[MAX_PATH] = { 0 };
			convert_slash(buf, sizeof(buf) - 1, dproj->rel_path.path, dproj->rel_path.len);
			fprintf_s(fp, " $(SLNDIR)/%.*s", dproj->rel_path.len, buf);
		}
	}

	fprintf_s(fp, "\n"
		      "\n"
		      ".PHONY: all clean depends");

	if (proj->all_depends.count > 0) {
		fprintf_s(fp, " $(DEPENDS)");
	}

	fprintf_s(fp, "\n"
		      "\n"
		      "all: $(BIN)");

	if (proj->all_depends.count > 0) {
		fprintf_s(fp, " $(DEPENDS)");
	}

	fprintf_s(fp, "\n"
		      "\n"
		      "$(BIN): $(OBJS)\n"
		      "\tar rcs $@ $^\n"
		      "\n"
		      "%%.o: %%.c\n"
		      "\t$(CC)");

	int include_diff = proj->props[PROJ_PROP_INCLUDE].set &&
			   (!proj->props[PROJ_PROP_SOURCE].set ||
			    !(proj->props[PROJ_PROP_INCLUDE].value.len == proj->props[PROJ_PROP_SOURCE].value.len &&
			      memcmp(proj->props[PROJ_PROP_SOURCE].value.data, proj->props[PROJ_PROP_INCLUDE].value.data, proj->props[PROJ_PROP_SOURCE].value.len) == 0));

	if (proj->props[PROJ_PROP_SOURCE].set) {
		char buf[MAX_PATH] = { 0 };
		convert_slash(buf, sizeof(buf) - 1, proj->props[PROJ_PROP_SOURCE].value.data, proj->props[PROJ_PROP_SOURCE].value.len);
		fprintf_s(fp, " -I%.*s", proj->props[PROJ_PROP_SOURCE].value.len, buf);
	}

	if (include_diff) {
		char buf[MAX_PATH] = { 0 };
		convert_slash(buf, sizeof(buf) - 1, proj->props[PROJ_PROP_INCLUDE].value.data, proj->props[PROJ_PROP_INCLUDE].value.len);
		fprintf_s(fp, " -I%.*s", proj->props[PROJ_PROP_INCLUDE].value.len, buf);
	}

	if (proj->props[PROJ_PROP_INCLUDES].set) {
		const array_t *includes = &proj->props[PROJ_PROP_INCLUDES].arr;

		for (int k = 0; k < includes->count; k++) {
			prop_str_t *include = array_get(includes, k);
			proj_t *iproj	    = { 0 };
			if (hashmap_get(projects, include->data, include->len, &iproj)) {
				ERR("project doesn't exists: '%.*s'", include->len, include->data);
				continue;
			}
			char buf[MAX_PATH] = { 0 };
			convert_slash(buf, sizeof(buf) - 1, iproj->rel_path.path, iproj->rel_path.len);
			fprintf_s(fp, " -I$(SLNDIR)/%.*s/%.*s", iproj->rel_path.len, buf, iproj->props[PROJ_PROP_INCLUDE].value.len,
				  iproj->props[PROJ_PROP_INCLUDE].value.data);
		}
	}

	fprintf_s(fp, " -c -o $@ $^\n"
		      "\n");

	if (proj->all_depends.count > 0) {
		fprintf_s(fp, "$(DEPENDS):\n"
			      "\t$(MAKE) -C $@\n"
			      "\n");
	}

	fprintf_s(fp, "clean:\n"
		      "\t$(RM) $(BIN) $(OBJS)\n");

	fclose(fp);
	if (ret == 0) {
		SUC("generating project: %s success", cmake_path.path);
	} else {
		ERR("generating project: %s failed", cmake_path.path);
	}

	return ret;
}
