#include "mk_proj.h"

#include "gen/var.h"

#include "defines.h"
#include "prop.h"
#include "utils.h"

static const var_pol_t vars = {
	.names = {
		[VAR_SLN_DIR] = "$(SLN_DIR)",
		[VAR_CONFIG] = "$(CONFIG)",
		[VAR_PLATFORM] = "$(PLATFORM)",
	},
	.tos = {
		[VAR_SLN_DIR] = "$(SLNDIR)",
		[VAR_CONFIG] = "Debug",
		[VAR_PLATFORM] = "x64",
	},
};

static inline void print_rel_path(FILE *fp, const proj_t *proj, const char *path, unsigned int path_len)
{
	char path_b[B_MAX_PATH] = { 0 };
	convert_slash(path_b, sizeof(path_b) - 1, path, path_len);
	char rel_path[B_MAX_PATH] = { 0 };
	convert_slash(rel_path, sizeof(rel_path) - 1, proj->rel_path.path, proj->rel_path.len);
	unsigned int rel_path_len = proj->rel_path.len;

	if (cstrn_cmp(path_b, path_len, "$(SLNDIR)", 9, 9)) {
		fprintf_s(fp, "%.*s", path_len, path_b);
	} else {
		fprintf_s(fp, "$(SLNDIR)/%.*s/%.*s", proj->rel_path.len, rel_path, path_len, path_b);
	}
}

int mk_proj_gen(const proj_t *proj, const hashmap_t *projects, const path_t *path, const prop_t *langs, const prop_t *outdir, const prop_t *intdir)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *sln_outdir = outdir;

	langs  = proj->props[PROJ_PROP_LANGS].set ? &proj->props[PROJ_PROP_LANGS] : langs;
	outdir = proj->props[PROJ_PROP_OUTDIR].set ? &proj->props[PROJ_PROP_OUTDIR] : outdir;
	intdir = proj->props[PROJ_PROP_INTDIR].set ? &proj->props[PROJ_PROP_INTDIR] : intdir;

	char buf[B_MAX_PATH]  = { 0 };
	char buf2[B_MAX_PATH] = { 0 };

	unsigned int buf_len;
	unsigned int buf2_len;

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

	const prop_str_t *inc = &proj->props[PROJ_PROP_INCLUDE].value;
	const prop_str_t *src = &proj->props[PROJ_PROP_SOURCE].value;
	unsigned int lang     = langs->mask;

	int deps_first = 0;

	if (outdir->set) {
		convert_slash(buf, sizeof(buf) - 1, outdir->value.data, outdir->value.len);
		buf2_len = cstr_replaces(buf, outdir->value.len, buf2, sizeof(buf2) - 1, vars.names, vars.tos, __VAR_MAX);
		buf_len	 = cstr_replace(buf2, buf2_len, buf, sizeof(buf) - 1, "$(PROJ_NAME)", 12, name->data, name->len);

		fprintf_s(fp, "OUTDIR = %.*s\n", buf_len, buf);
	}

	if (intdir->set) {
		convert_slash(buf, sizeof(buf) - 1, intdir->value.data, intdir->value.len);
		buf2_len = cstr_replaces(buf, intdir->value.len, buf2, sizeof(buf2) - 1, vars.names, vars.tos, __VAR_MAX);
		buf_len	 = cstr_replace(buf2, buf2_len, buf, sizeof(buf) - 1, "$(PROJ_NAME)", 12, name->data, name->len);

		fprintf_s(fp, "INTDIR = %.*s\n", buf_len, buf);
	}

	if (proj->props[PROJ_PROP_INCLUDE].set) {
		if (lang & (1 << LANG_C) || lang & (1 << LANG_CPP)) {
			fprintf_s(fp, "DEPS %.*s= $(shell find %.*s -name '*.h')\n", deps_first, "+", inc->len, inc->data);
			deps_first = 1;
		}

		if (lang & (1 << LANG_CPP)) {
			fprintf_s(fp, "DEPS %.*s= $(shell find %.*s -name '*.hpp')\n", deps_first, "+", inc->len, inc->data);
			deps_first = 1;
		}
	}

	if (proj->props[PROJ_PROP_SOURCE].set) {
		if (lang & (1 << LANG_C) || lang & (1 << LANG_CPP)) {
			fprintf_s(fp, "DEPS %.*s= $(shell find %.*s -name '*.h')\n", deps_first, "+", src->len, src->data);
			deps_first = 1;
		}

		if (lang & (1 << LANG_CPP)) {
			fprintf_s(fp, "DEPS %.*s= $(shell find %.*s -name '*.hpp')\n", deps_first, "+", src->len, src->data);
			deps_first = 1;
		}

		if (lang & (1 << LANG_C)) {
			fprintf_s(fp, "SRC_C = $(shell find %.*s -name '*.c')\n", src->len, src->data);
		}

		if (lang & (1 << LANG_CPP)) {
			fprintf_s(fp, "SRC_CPP = $(shell find %.*s -name '*.cpp')\n", src->len, src->data);
		}
	}

	if (lang & (1 << LANG_C)) {
		fprintf_s(fp, "OBJ_C = $(patsubst %%.c, $(INTDIR)%%.o, $(SRC_C))\n");
	}

	if (lang & (1 << LANG_CPP)) {
		fprintf_s(fp, "OBJ_CPP = $(patsubst %%.cpp, $(INTDIR)%%.o, $(SRC_CPP))\n");
	}

	switch (type) {
	case PROJ_TYPE_EXE:
		fprintf_s(fp, "TARGET = $(OUTDIR)%.*s\n\n", name->len, name->data);
		break;
	default:
		fprintf_s(fp, "TARGET = $(OUTDIR)%.*s.a\n\n", name->len, name->data);
		break;
	}

	fprintf_s(fp, "FLAGS =");

	int include_diff = proj->props[PROJ_PROP_INCLUDE].set &&
			   (!proj->props[PROJ_PROP_SOURCE].set || !prop_cmp(&proj->props[PROJ_PROP_INCLUDE].value, &proj->props[PROJ_PROP_SOURCE].value));

	if (proj->props[PROJ_PROP_SOURCE].set) {
		convert_slash(buf, sizeof(buf) - 1, proj->props[PROJ_PROP_SOURCE].value.data, proj->props[PROJ_PROP_SOURCE].value.len);
		fprintf_s(fp, " -I%.*s", proj->props[PROJ_PROP_SOURCE].value.len, buf);
	}

	if (include_diff) {
		convert_slash(buf, sizeof(buf) - 1, proj->props[PROJ_PROP_INCLUDE].value.data, proj->props[PROJ_PROP_INCLUDE].value.len);
		fprintf_s(fp, " -I%.*s", proj->props[PROJ_PROP_INCLUDE].value.len, buf);
	}

	for (int i = 0; i < proj->includes.count; i++) {
		const proj_t *iproj = *(proj_t **)array_get(&proj->includes, i);

		if (iproj->props[PROJ_PROP_INCLUDE].set) {
			convert_slash(buf, sizeof(buf) - 1, iproj->rel_path.path, iproj->rel_path.len);
			fprintf_s(fp, " -I$(SLNDIR)/%.*s/%.*s", iproj->rel_path.len, buf, iproj->props[PROJ_PROP_INCLUDE].value.len,
				  iproj->props[PROJ_PROP_INCLUDE].value.data);
		}

		if (iproj->props[PROJ_PROP_ENCLUDE].set) {
			buf_len	 = cstr_replaces(iproj->props[PROJ_PROP_ENCLUDE].value.data, iproj->props[PROJ_PROP_ENCLUDE].value.len, buf, sizeof(buf) - 1, vars.names,
						 vars.tos, __VAR_MAX);
			buf2_len = cstr_replace(buf, buf_len, buf2, sizeof(buf2) - 1, "$(PROJ_NAME)", 12, iproj->name->data, iproj->name->len);

			fprintf_s(fp, " -I");
			print_rel_path(fp, iproj, buf2, buf2_len);
		}
	}

	fprintf_s(fp, "\n");

	fprintf_s(fp, "CFLAGS += $(FLAGS)\n");

	if (lang & (1 << LANG_CPP)) {
		fprintf_s(fp, "CXXFLAGS += $(FLAGS)\n");
	}

	if (type == PROJ_TYPE_EXE) {
		fprintf_s(fp, "LDFLAGS += -static");

		for (int i = 0; i < proj->all_depends.count; i++) {
			const proj_t *dproj = *(proj_t **)array_get(&proj->all_depends, i);

			if (dproj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
				const prop_t *doutdir = dproj->props[PROJ_PROP_OUTDIR].set ? &dproj->props[PROJ_PROP_OUTDIR] : sln_outdir;

				if (doutdir->set) {
					convert_slash(buf, sizeof(buf) - 1, doutdir->value.data, doutdir->value.len);
					buf2_len = cstr_replaces(buf, doutdir->value.len, buf2, sizeof(buf2) - 1, vars.names, vars.tos, __VAR_MAX);
					buf_len	 = cstr_replace(buf2, buf2_len, buf, sizeof(buf) - 1, "$(PROJ_NAME)", 12, dproj->name->data, dproj->name->len);

					fprintf_s(fp, " -L");
					print_rel_path(fp, dproj, buf, buf_len);

					fprintf_s(fp, " -l:%.*s.a", dproj->name->len, dproj->name->data);
				} else {
					convert_slash(buf, sizeof(buf) - 1, dproj->rel_path.path, dproj->rel_path.len);
					fprintf_s(fp, " -L$(SLNDIR)/%.*s -l:%.*s.a", dproj->rel_path.len, buf, dproj->name->len, dproj->name->data);
				}
			}

			if (dproj->props[PROJ_PROP_LIBDIRS].set) {
				const array_t *libdirs = &dproj->props[PROJ_PROP_LIBDIRS].arr;
				for (int j = 0; j < libdirs->count; j++) {
					prop_str_t *libdir = array_get(libdirs, j);
					if (libdir->len > 0) {
						buf_len	 = cstr_replaces(libdir->data, libdir->len, buf, sizeof(buf) - 1, vars.names, vars.tos, __VAR_MAX);
						buf2_len = cstr_replace(buf, buf_len, buf2, sizeof(buf2) - 1, "$(PROJ_NAME)", 12, dproj->name->data, dproj->name->len);

						fprintf_s(fp, " -L");
						print_rel_path(fp, dproj, buf2, buf2_len);
					}
				}
			}

			if (dproj->props[PROJ_PROP_LINK].set) {
				const array_t *links = &dproj->props[PROJ_PROP_LINK].arr;
				for (int j = 0; j < links->count; j++) {
					prop_str_t *link = array_get(links, j);
					if (link->len > 0) {
						fprintf_s(fp, " -l:%.*s.a", link->len, link->data);
					}
				}
			}
		}

		if (lang & (1 << LANG_CPP)) {
			fprintf_s(fp, " -lstdc++");
		}

		fprintf_s(fp, "\n");
	}

	fprintf_s(fp, "\n");

	fprintf_s(fp, "all: %.*s", name->len, name->data);

	fprintf_s(fp, "\n\n");

	fprintf_s(fp, "%.*s: $(TARGET)", name->len, name->data);

	fprintf_s(fp, "\n\n");

	fprintf_s(fp, "$(TARGET):");

	if (lang & (1 << LANG_C)) {
		fprintf_s(fp, " $(OBJ_C)");
	}

	if (lang & (1 << LANG_CPP)) {
		fprintf_s(fp, " $(OBJ_CPP)");
	}

	fprintf_s(fp, "\n\tmkdir -p $(@D)\n");

	switch (type) {
	case PROJ_TYPE_LIB:
		fprintf_s(fp, "\tar rcs $@ $^\n");
		break;
	case PROJ_TYPE_EXE:
		fprintf_s(fp, "\tcc -o $@ $^ $(LDFLAGS)\n");
		break;
	}

	fprintf_s(fp, "\n");

	if (lang & (1 << LANG_C)) {
		fprintf_s(fp, "$(INTDIR)%%.o: %%.c\n"
			      "\tmkdir -p $(@D)\n"
			      "\t$(CC) $(CFLAGS) -c -o $@ $<\n\n");
	}

	if (lang & (1 << LANG_CPP)) {
		fprintf_s(fp, "$(INTDIR)%%.o: %%.cpp\n"
			      "\tmkdir -p $(@D)\n"
			      "\t$(CC) $(CXXFLAGS) -c -o $@ $<\n\n");
	}

	fprintf_s(fp, ".PHONY: all %.*s clean\n\n", name->len, name->data);

	fprintf_s(fp, "clean:\n"
		      "\t$(RM) $(TARGET)");

	if (lang & (1 << LANG_C)) {
		fprintf_s(fp, " $(OBJ_C)");
	}

	if (lang & (1 << LANG_CPP)) {
		fprintf_s(fp, " $(OBJ_CPP)");
	}

	fprintf_s(fp, "\n");

	fclose(fp);
	if (ret == 0) {
		SUC("generating project: %s success", cmake_path.path);
	} else {
		ERR("generating project: %s failed", cmake_path.path);
	}

	return ret;
}
