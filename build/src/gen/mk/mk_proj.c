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

static size_t resolve(const prop_str_t *prop, char *dst, size_t dst_max_len, const proj_t *proj)
{
	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len, dst_len;

	buf_len = convert_slash(CSTR(buf), prop->data, prop->len);
	dst_len = cstr_replaces(buf, buf_len, dst, dst_max_len, vars.names, vars.tos, __VAR_MAX);
	buf_len = cstr_replace(dst, dst_len, CSTR(buf), CSTR("$(PROJ_NAME)"), proj->name->data, proj->name->len);
	dst_len = cstr_replace(buf, buf_len, dst, dst_max_len, CSTR("$(PROJ_FOLDER)"), proj->rel_path.path, proj->rel_path.len);

	return dst_len;
}

static inline void print_rel_path(FILE *fp, const proj_t *proj, const char *path, size_t path_len)
{
	char path_b[P_MAX_PATH]	  = { 0 };
	size_t path_b_len	  = convert_slash(CSTR(path_b), path, path_len);
	char rel_path[P_MAX_PATH] = { 0 };
	size_t rel_path_len	  = convert_slash(CSTR(rel_path), proj->rel_path.path, proj->rel_path.len);

	if (cstrn_cmp(path_b, path_b_len, CSTR("$(SLNDIR)"), 9)) {
		p_fprintf(fp, "%.*s", path_b_len, path_b);
	} else {
		p_fprintf(fp, "$(SLNDIR)/%.*s/%.*s", rel_path_len, rel_path, path_b_len, path_b);
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

	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len;

	int ret = 0;

	path_t cmake_path = *path;
	if (path_child(&cmake_path, proj->rel_path.path, proj->rel_path.len)) {
		return 1;
	}

	if (!folder_exists(cmake_path.path)) {
		folder_create(cmake_path.path);
	}

	if (path_child(&cmake_path, CSTR("Makefile"))) {
		return 1;
	}

	FILE *fp = file_open(cmake_path.path, "w");
	if (fp == NULL) {
		return 1;
	}

	MSG("generating project: %s", cmake_path.path);

	uint lang = langs->mask;

	int deps_first = 0;

	if (outdir->flags & PROP_SET) {
		buf_len = resolve(&outdir->value, CSTR(buf), proj);
		p_fprintf(fp, "OUTDIR = %.*s\n", buf_len, buf);
	}

	if (intdir->flags & PROP_SET) {
		buf_len = resolve(&intdir->value, CSTR(buf), proj);
		p_fprintf(fp, "INTDIR = %.*s\n", buf_len, buf);
	}

	p_fprintf(fp, "REPDIR = $(OUTDIR)coverage-report/\n");

	if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
		const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;

		for (uint i = 0; i < includes->cnt; i++) {
			prop_str_t *include = arr_get(includes, i);

			if (lang & (1 << LANG_C) || lang & (1 << LANG_CPP)) {
				p_fprintf(fp, "DEPS %.*s= $(shell find %.*s -name '*.h')\n", deps_first, "+", include->len, include->data);
				deps_first = 1;
			}

			if (lang & (1 << LANG_CPP)) {
				p_fprintf(fp, "DEPS %.*s= $(shell find %.*s -name '*.hpp')\n", deps_first, "+", include->len, include->data);
				deps_first = 1;
			}
		}
	}

	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;

		for (uint i = 0; i < sources->cnt; i++) {
			prop_str_t *source = arr_get(sources, i);

			if (lang & (1 << LANG_C) || lang & (1 << LANG_CPP)) {
				p_fprintf(fp, "DEPS %.*s= $(shell find %.*s -name '*.h')\n", deps_first, "+", source->len, source->data);
				deps_first = 1;
			}

			if (lang & (1 << LANG_CPP)) {
				p_fprintf(fp, "DEPS %.*s= $(shell find %.*s -name '*.hpp')\n", deps_first, "+", source->len, source->data);
				deps_first = 1;
			}

			if (lang & (1 << LANG_C)) {
				p_fprintf(fp, "SRC_C = $(shell find %.*s -name '*.c')\n", source->len, source->data);
			}

			if (lang & (1 << LANG_CPP)) {
				p_fprintf(fp, "SRC_CPP = $(shell find %.*s -name '*.cpp')\n", source->len, source->data);
			}
		}
	}

	if (lang & (1 << LANG_C)) {
		p_fprintf(fp, "OBJ_C = $(patsubst %%.c, $(INTDIR)%%.o, $(SRC_C))\n");
	}

	if (lang & (1 << LANG_CPP)) {
		p_fprintf(fp, "OBJ_CPP = $(patsubst %%.cpp, $(INTDIR)%%.o, $(SRC_CPP))\n");
	}

	p_fprintf(fp, "LCOV = $(OUTDIR)lcov.info\n");

	bool cov = 0;
	if (lang & (1 << LANG_C)) {
		p_fprintf(fp, "COV %.*s= $(patsubst %%.c, $(INTDIR)%%.gcno, $(SRC_C))\n", cov, "+");
		p_fprintf(fp, "COV += $(patsubst %%.c, $(INTDIR)%%.gcda, $(SRC_C))\n", cov, "+");
		cov = 1;
	}

	if (lang & (1 << LANG_CPP)) {
		p_fprintf(fp, "COV %.*s= $(patsubst %%.cpp, $(INTDIR)%%.gcno, $(SRC_CPP))\n", cov, "+");
		p_fprintf(fp, "COV += $(patsubst %%.cpp, $(INTDIR)%%.gcda, $(SRC_CPP))\n", cov, "+");
		cov = 1;
	}

	p_fprintf(fp, "COV %.*s= $(LCOV) $(REPDIR)\n", cov, "+");

	switch (type) {
	case PROJ_TYPE_EXE:
		p_fprintf(fp, "TARGET = $(OUTDIR)%.*s\n\n", name->len, name->data);
		break;
	default:
		p_fprintf(fp, "TARGET = $(OUTDIR)%.*s.a\n\n", name->len, name->data);
		break;
	}

	p_fprintf(fp, "FLAGS =");

	//TODO: Remove
	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;

		for (uint i = 0; i < sources->cnt; i++) {
			prop_str_t *source = arr_get(sources, i);

			buf_len = resolve(source, CSTR(buf), proj);
			p_fprintf(fp, " -I%.*s", buf_len, buf);
		}
	}

	if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
		const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;

		for (uint i = 0; i < includes->cnt; i++) {
			prop_str_t *include = arr_get(includes, i);

			buf_len = resolve(include, CSTR(buf), proj);
			p_fprintf(fp, " -I%.*s", buf_len, buf);
		}
	}

	for (uint i = 0; i < proj->includes.cnt; i++) {
		const proj_t *iproj = *(proj_t **)arr_get(&proj->includes, i);

		if (iproj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
			const arr_t *includes = &iproj->props[PROJ_PROP_INCLUDE].arr;

			for (uint i = 0; i < includes->cnt; i++) {
				prop_str_t *include = arr_get(includes, i);

				char rel_path[P_MAX_PATH] = { 0 };
				size_t rel_path_len;

				buf_len	     = resolve(include, CSTR(buf), iproj);
				rel_path_len = convert_slash(CSTR(rel_path), iproj->rel_path.path, iproj->rel_path.len);
				p_fprintf(fp, " -I$(SLNDIR)/%.*s/%.*s", rel_path_len, rel_path, buf_len, buf);
			}
		}

		if (iproj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
			buf_len = resolve(&iproj->props[PROJ_PROP_ENCLUDE].value, CSTR(buf), iproj);
			p_fprintf(fp, " -I");
			print_rel_path(fp, iproj, buf, buf_len);
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

		for (int i = proj->all_depends.cnt - 1; i >= 0; i--) {
			const proj_t *dproj = *(proj_t **)arr_get(&proj->all_depends, i);

			if (dproj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
				const prop_t *doutdir = &dproj->props[PROJ_PROP_OUTDIR];

				if (doutdir->flags & PROP_SET) {
					buf_len = resolve(&doutdir->value, CSTR(buf), dproj);
					p_fprintf(fp, " -L");
					print_rel_path(fp, dproj, buf, buf_len);

					p_fprintf(fp, " -l:%.*s.a", dproj->name->len, dproj->name->data);
				} else {
					buf_len = convert_slash(CSTR(buf), dproj->rel_path.path, dproj->rel_path.len);
					p_fprintf(fp, " -L$(SLNDIR)/%.*s -l:%.*s.a", buf_len, buf, dproj->name->len, dproj->name->data);
				}
			}

			if (dproj->props[PROJ_PROP_LIBDIRS].flags & PROP_SET) {
				const arr_t *libdirs = &dproj->props[PROJ_PROP_LIBDIRS].arr;
				for (uint j = 0; j < libdirs->cnt; j++) {
					prop_str_t *libdir = arr_get(libdirs, j);
					if (libdir->len > 0) {
						buf_len = resolve(libdir, CSTR(buf), dproj);
						p_fprintf(fp, " -L");
						print_rel_path(fp, dproj, buf, buf_len);
					}
				}
			}

			if (dproj->props[PROJ_PROP_LINK].flags & PROP_SET) {
				const arr_t *links = &dproj->props[PROJ_PROP_LINK].arr;
				for (uint j = 0; j < links->cnt; j++) {
					prop_str_t *link = arr_get(links, j);
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

	p_fprintf(fp, "\nRM += -r\n");

	p_fprintf(fp, "\nCONFIG_FLAGS =\n\n");

	if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE) {
		p_fprintf(fp, "SHOW = true\n\n");
	}

	p_fprintf(fp,
		  ".PHONY: all check check_coverage %.*s compile coverage clean\n\n"
		  "all: %.*s\n\ncheck:\n",
		  name->len, name->data, name->len, name->data);

	if ((configs->flags & PROP_SET) && configs->arr.cnt > 0) {
		p_fprintf(fp, "ifeq ($(CONFIG), Debug)\n"
			      "\t$(eval CONFIG_FLAGS += -ggdb3 -O0)\n"
			      "endif\n");
	}

	p_fprintf(fp,
		  "\ncheck_coverage: check\n"
		  "\t$(eval CONFIG_FLAGS += --coverage -fprofile-abs-path)\n"
		  "ifeq (, $(shell which lcov))\n"
		  "\t$(error \"lcov not found\")\n"
		  "endif\n"
		  "\n%.*s: clean compile\n"
		  "\ncompile: check $(TARGET)\n"
		  "\ncoverage: clean check_coverage $(TARGET)\n",
		  name->len, name->data, name->len, name->data);

	if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE) {
		p_fprintf(fp, "\t@$(TARGET) $(ARGS)\n"
			      "\t@lcov -q -c -d $(SLNDIR) -o $(LCOV)\n"
			      "ifeq ($(SHOW), true)\n"
			      "\t@genhtml -q $(LCOV) -o $(REPDIR)\n"
			      "\t@open $(REPDIR)index.html\n"
			      "endif\n");
	}

	p_fprintf(fp, "\n$(TARGET):");

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

	if (lang & (((1 << LANG_C) | (1 << LANG_CPP)))) {
		p_fprintf(fp, " $(COV)");
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
