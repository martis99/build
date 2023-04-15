#include "mk_proj.h"

#include "gen/sln.h"
#include "gen/var.h"

#include "common.h"

static const var_pol_t vars = {
	.names = {
		[VAR_SLN_DIR] = "$(SLN_DIR)",
		[VAR_CONFIG] = "$(CONFIG)",
		[VAR_PLATFORM] = "$(PLATFORM)",
	},
	.tos = {
		[VAR_SLN_DIR] = "$(SLNDIR)",
		[VAR_CONFIG] = "$(CONFIG)",
		[VAR_PLATFORM] = "x64",
	},
};

static inline void print_rel_path(FILE *fp, const proj_t *proj, const char *path, size_t path_len)
{
	char path_b[P_MAX_PATH] = { 0 };
	convert_slash(path_b, sizeof(path_b) - 1, path, path_len);
	char rel_path[P_MAX_PATH] = { 0 };
	convert_slash(rel_path, sizeof(rel_path) - 1, proj->rel_path.path, proj->rel_path.len);
	size_t rel_path_len = proj->rel_path.len;

	if (cstrn_cmp(path_b, path_len, "$(SLNDIR)", 9, 9)) {
		p_fprintf(fp, "%.*s", path_len, path_b);
	} else {
		p_fprintf(fp, "$(SLNDIR)/%.*s/%.*s", proj->rel_path.len, rel_path, path_len, path_b);
	}
}

int mk_proj_gen(const proj_t *proj, const hashmap_t *projects, const path_t *path, const prop_t *sln_props)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *configs = &sln_props[SLN_PROP_CONFIGS];
	const prop_t *langs   = &proj->props[PROJ_PROP_LANGS];
	const prop_t *cflags  = &proj->props[PROJ_PROP_CFLAGS];
	const prop_t *outdir  = &proj->props[PROJ_PROP_OUTDIR];
	const prop_t *intdir  = &proj->props[PROJ_PROP_INTDIR];

	char buf[P_MAX_PATH]  = { 0 };
	char buf2[P_MAX_PATH] = { 0 };

	size_t buf_len;
	size_t buf2_len;

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

	FILE *fp = file_open(cmake_path.path, "w");
	if (fp == NULL) {
		return 1;
	}

	MSG("generating project: %s", cmake_path.path);

	const prop_str_t *inc = &proj->props[PROJ_PROP_INCLUDE].value;
	const prop_str_t *src = &proj->props[PROJ_PROP_SOURCE].value;
	uint lang	      = langs->mask;

	int deps_first = 0;

	if (outdir->flags & PROP_SET) {
		convert_slash(buf, sizeof(buf) - 1, outdir->value.data, outdir->value.len);
		buf2_len = cstr_replaces(buf, outdir->value.len, buf2, sizeof(buf2) - 1, vars.names, vars.tos, __VAR_MAX);
		buf_len	 = cstr_replace(buf2, buf2_len, buf, sizeof(buf) - 1, "$(PROJ_NAME)", 12, name->data, name->len);

		p_fprintf(fp, "OUTDIR = %.*s\n", buf_len, buf);
	}

	if (intdir->flags & PROP_SET) {
		convert_slash(buf, sizeof(buf) - 1, intdir->value.data, intdir->value.len);
		buf2_len = cstr_replaces(buf, intdir->value.len, buf2, sizeof(buf2) - 1, vars.names, vars.tos, __VAR_MAX);
		buf_len	 = cstr_replace(buf2, buf2_len, buf, sizeof(buf) - 1, "$(PROJ_NAME)", 12, name->data, name->len);

		p_fprintf(fp, "INTDIR = %.*s\n", buf_len, buf);
	}

	if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
		if (lang & (1 << LANG_C) || lang & (1 << LANG_CPP)) {
			p_fprintf(fp, "DEPS %.*s= $(shell find %.*s -name '*.h')\n", deps_first, "+", inc->len, inc->data);
			deps_first = 1;
		}

		if (lang & (1 << LANG_CPP)) {
			p_fprintf(fp, "DEPS %.*s= $(shell find %.*s -name '*.hpp')\n", deps_first, "+", inc->len, inc->data);
			deps_first = 1;
		}
	}

	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		if (lang & (1 << LANG_C) || lang & (1 << LANG_CPP)) {
			p_fprintf(fp, "DEPS %.*s= $(shell find %.*s -name '*.h')\n", deps_first, "+", src->len, src->data);
			deps_first = 1;
		}

		if (lang & (1 << LANG_CPP)) {
			p_fprintf(fp, "DEPS %.*s= $(shell find %.*s -name '*.hpp')\n", deps_first, "+", src->len, src->data);
			deps_first = 1;
		}

		if (lang & (1 << LANG_C)) {
			p_fprintf(fp, "SRC_C = $(shell find %.*s -name '*.c')\n", src->len, src->data);
		}

		if (lang & (1 << LANG_CPP)) {
			p_fprintf(fp, "SRC_CPP = $(shell find %.*s -name '*.cpp')\n", src->len, src->data);
		}
	}

	if (lang & (1 << LANG_C)) {
		p_fprintf(fp, "OBJ_C = $(patsubst %%.c, $(INTDIR)%%.o, $(SRC_C))\n");
	}

	if (lang & (1 << LANG_CPP)) {
		p_fprintf(fp, "OBJ_CPP = $(patsubst %%.cpp, $(INTDIR)%%.o, $(SRC_CPP))\n");
	}

	switch (type) {
	case PROJ_TYPE_EXE:
		p_fprintf(fp, "TARGET = $(OUTDIR)%.*s\n\n", name->len, name->data);
		break;
	default:
		p_fprintf(fp, "TARGET = $(OUTDIR)%.*s.a\n\n", name->len, name->data);
		break;
	}

	p_fprintf(fp, "FLAGS =");

	int include_diff = (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) &&
			   (!(proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) || !prop_cmp(&proj->props[PROJ_PROP_INCLUDE].value, &proj->props[PROJ_PROP_SOURCE].value));

	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		convert_slash(buf, sizeof(buf) - 1, proj->props[PROJ_PROP_SOURCE].value.data, proj->props[PROJ_PROP_SOURCE].value.len);
		p_fprintf(fp, " -I%.*s", proj->props[PROJ_PROP_SOURCE].value.len, buf);
	}

	if (include_diff) {
		convert_slash(buf, sizeof(buf) - 1, proj->props[PROJ_PROP_INCLUDE].value.data, proj->props[PROJ_PROP_INCLUDE].value.len);
		p_fprintf(fp, " -I%.*s", proj->props[PROJ_PROP_INCLUDE].value.len, buf);
	}

	for (int i = 0; i < proj->includes.count; i++) {
		const proj_t *iproj = *(proj_t **)array_get(&proj->includes, i);

		if (iproj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
			convert_slash(buf, sizeof(buf) - 1, iproj->rel_path.path, iproj->rel_path.len);
			p_fprintf(fp, " -I$(SLNDIR)/%.*s/%.*s", iproj->rel_path.len, buf, iproj->props[PROJ_PROP_INCLUDE].value.len,
				  iproj->props[PROJ_PROP_INCLUDE].value.data);
		}

		if (iproj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
			buf_len	 = cstr_replaces(iproj->props[PROJ_PROP_ENCLUDE].value.data, iproj->props[PROJ_PROP_ENCLUDE].value.len, buf, sizeof(buf) - 1, vars.names,
						 vars.tos, __VAR_MAX);
			buf2_len = cstr_replace(buf, buf_len, buf2, sizeof(buf2) - 1, "$(PROJ_NAME)", 12, iproj->name->data, iproj->name->len);

			p_fprintf(fp, " -I");
			print_rel_path(fp, iproj, buf2, buf2_len);
		}
	}

	p_fprintf(fp, "\n");

	if (lang & (1 << LANG_C)) {
		p_fprintf(fp, "CFLAGS += $(FLAGS)");

		if ((cflags->flags & PROP_SET) && cflags->mask & (1 << CFLAG_STD_C99)) {
			p_fprintf(fp, " -std=c99");
		}
		p_fprintf(fp, "\n");
	}

	if (lang & (1 << LANG_CPP)) {
		p_fprintf(fp, "CXXFLAGS += $(FLAGS)\n");
	}

	if (type == PROJ_TYPE_EXE) {
		p_fprintf(fp, "LDFLAGS +=");

		const prop_t *ldflags = &proj->props[PROJ_PROP_LDFLAGS];

		if (ldflags->flags & PROP_SET) {
			if (ldflags->mask & (1 << LDFLAG_WHOLEARCHIVE)) {
				p_fprintf(fp, " -Wl,--whole-archive");
			}
			if (ldflags->mask & (1 << LDFLAG_ALLOWMULTIPLEDEFINITION)) {
				p_fprintf(fp, " -Wl,--allow-multiple-definition");
			}
		}

		for (int i = proj->all_depends.count - 1; i >= 0; i--) {
			const proj_t *dproj = *(proj_t **)array_get(&proj->all_depends, i);

			if (dproj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
				const prop_t *doutdir = &dproj->props[PROJ_PROP_OUTDIR];

				if (doutdir->flags & PROP_SET) {
					convert_slash(buf, sizeof(buf) - 1, doutdir->value.data, doutdir->value.len);
					buf2_len = cstr_replaces(buf, doutdir->value.len, buf2, sizeof(buf2) - 1, vars.names, vars.tos, __VAR_MAX);
					buf_len	 = cstr_replace(buf2, buf2_len, buf, sizeof(buf) - 1, "$(PROJ_NAME)", 12, dproj->name->data, dproj->name->len);

					p_fprintf(fp, " -L");
					print_rel_path(fp, dproj, buf, buf_len);

					p_fprintf(fp, " -l:%.*s.a", dproj->name->len, dproj->name->data);
				} else {
					convert_slash(buf, sizeof(buf) - 1, dproj->rel_path.path, dproj->rel_path.len);
					p_fprintf(fp, " -L$(SLNDIR)/%.*s -l:%.*s.a", dproj->rel_path.len, buf, dproj->name->len, dproj->name->data);
				}
			}

			if (dproj->props[PROJ_PROP_LIBDIRS].flags & PROP_SET) {
				const array_t *libdirs = &dproj->props[PROJ_PROP_LIBDIRS].arr;
				for (int j = 0; j < libdirs->count; j++) {
					prop_str_t *libdir = array_get(libdirs, j);
					if (libdir->len > 0) {
						buf_len	 = cstr_replaces(libdir->data, libdir->len, buf, sizeof(buf) - 1, vars.names, vars.tos, __VAR_MAX);
						buf2_len = cstr_replace(buf, buf_len, buf2, sizeof(buf2) - 1, "$(PROJ_NAME)", 12, dproj->name->data, dproj->name->len);

						p_fprintf(fp, " -L");
						print_rel_path(fp, dproj, buf2, buf2_len);
					}
				}
			}

			if (dproj->props[PROJ_PROP_LINK].flags & PROP_SET) {
				const array_t *links = &dproj->props[PROJ_PROP_LINK].arr;
				for (int j = 0; j < links->count; j++) {
					prop_str_t *link = array_get(links, j);
					if (link->len > 0) {
						p_fprintf(fp, " -l:%.*s.a", link->len, link->data);
					}
				}
			}
		}

		if ((ldflags->flags & PROP_SET) && (ldflags->mask & (1 << LDFLAG_WHOLEARCHIVE))) {
			p_fprintf(fp, " -Wl,--no-whole-archive");
		}

		if (lang & (1 << LANG_CPP)) {
			p_fprintf(fp, " -lstdc++");
		}

		if (ldflags->flags & PROP_SET) {
			if (ldflags->mask & (1 << LDFLAG_MATH)) {
				p_fprintf(fp, " -lm");
			}

			if (ldflags->mask & (1 << LDFLAG_X11)) {
				p_fprintf(fp, " -lX11");
			}

			if (ldflags->mask & (1 << LDFLAG_GL)) {
				p_fprintf(fp, " -lGL");
			}

			if (ldflags->mask & (1 << LDFLAG_GLX)) {
				p_fprintf(fp, " -lGLX");
			}
		}

		p_fprintf(fp, "\n");
	}

	p_fprintf(fp, "\n");

	p_fprintf(fp, ".PHONY: all check %.*s clean\n\n", name->len, name->data);

	p_fprintf(fp, "all: %.*s", name->len, name->data);

	p_fprintf(fp, "\n\n");

	p_fprintf(fp, "check:\n");

	if ((configs->flags & PROP_SET) && configs->arr.count > 0) {
		p_fprintf(fp, "ifeq ($(CONFIG), Debug)\n"
			      "CONFIG_FLAGS = -ggdb3 -O0\n"
			      "endif\n");
	}

	p_fprintf(fp, "\n");

	p_fprintf(fp, "%.*s: check $(TARGET)", name->len, name->data);

	p_fprintf(fp, "\n\n");

	p_fprintf(fp, "$(TARGET):");

	if (lang & (1 << LANG_C)) {
		p_fprintf(fp, " $(OBJ_C)");
	}

	if (lang & (1 << LANG_CPP)) {
		p_fprintf(fp, " $(OBJ_CPP)");
	}

	p_fprintf(fp, "\n\t@mkdir -p $(@D)\n");

	switch (type) {
	case PROJ_TYPE_LIB:
		p_fprintf(fp, "\t@ar rcs $@ $^\n");
		break;
	case PROJ_TYPE_EXE:
		p_fprintf(fp, "\t@$(CC) $(CONFIG_FLAGS) -o $@ $^ $(LDFLAGS)\n");
		break;
	}

	p_fprintf(fp, "\n");

	if (lang & (1 << LANG_C)) {
		p_fprintf(fp, "$(INTDIR)%%.o: %%.c\n"
			      "\t@mkdir -p $(@D)\n"
			      "\t@$(CC) $(CONFIG_FLAGS) $(CFLAGS) -c -o $@ $<\n\n");
	}

	if (lang & (1 << LANG_CPP)) {
		p_fprintf(fp, "$(INTDIR)%%.o: %%.cpp\n"
			      "\t@mkdir -p $(@D)\n"
			      "\t@$(CC) $(CONFIG_FLAGS) $(CXXFLAGS) -c -o $@ $<\n\n");
	}

	p_fprintf(fp, "clean:\n"
		      "\t@$(RM) $(TARGET)");

	if (lang & (1 << LANG_C)) {
		p_fprintf(fp, " $(OBJ_C)");
	}

	if (lang & (1 << LANG_CPP)) {
		p_fprintf(fp, " $(OBJ_CPP)");
	}

	p_fprintf(fp, "\n");

	file_close(fp);
	if (ret == 0) {
		SUC("generating project: %s success", cmake_path.path);
	} else {
		ERR("generating project: %s failed", cmake_path.path);
	}

	return ret;
}
