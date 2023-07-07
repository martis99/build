#include "mk_proj.h"

#include "gen/sln.h"
#include "gen/var.h"

#include "common.h"

static const var_pol_t vars = {
	.old = {
		[VAR_SLN_DIR] = "$(SLN_DIR)",
		[VAR_CONFIG] = "$(CONFIG)",
		[VAR_PLATFORM] = "$(PLATFORM)",
	},
	.new = {
		[VAR_SLN_DIR] = "$(SLNDIR)",
		[VAR_CONFIG] = "$(CONFIG)",
		[VAR_PLATFORM] = "$(PLATFORM)",
	},
};

static size_t resolve(const prop_str_t *prop, char *dst, size_t dst_size, const proj_t *proj)
{
	size_t dst_len = prop->len;
	m_memcpy(CSTR(dst), prop->data, prop->len);

	dst_len = invert_slash(dst, dst_len);
	dst_len = cstr_inplaces(dst, dst_size, dst_len, vars.old, vars.new, __VAR_MAX);
	dst_len = cstr_inplace(dst, dst_size, dst_len, CSTR("$(PROJ_NAME)"), proj->name->data, proj->name->len);
	dst_len = cstr_inplace(dst, dst_size, dst_len, CSTR("$(PROJ_FOLDER)"), proj->rel_path.path, proj->rel_path.len);

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

static int gen_source(const proj_t *proj, const hashmap_t *projects, const prop_t *sln_props, FILE *fp)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *configs  = &sln_props[SLN_PROP_CONFIGS];
	const prop_t *langs    = &proj->props[PROJ_PROP_LANGS];
	const prop_t *cflags   = &proj->props[PROJ_PROP_CFLAGS];
	const prop_t *outdir   = &proj->props[PROJ_PROP_OUTDIR];
	const prop_t *intdir   = &proj->props[PROJ_PROP_INTDIR];
	const prop_t *depends  = &proj->props[PROJ_PROP_DEPENDS];
	const prop_t *requires = &proj->props[PROJ_PROP_REQUIRE];

	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len;

	uint lang = langs->mask;

	int deps_first = 0;

	int ret = 0;

	if (intdir->flags & PROP_SET) {
		buf_len = resolve(&intdir->value, CSTR(buf), proj);
		p_fprintf(fp, "INTDIR = %.*s\n", buf_len, buf);
	}

	p_fprintf(fp, "REPDIR = $(OUTDIR)coverage-report/\n");

	if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
		const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;

		for (uint i = 0; i < includes->cnt; i++) {
			prop_str_t *include = arr_get(includes, i);

			if (lang & (1 << LANG_ASM)) {
				p_fprintf(fp, "DEPS %.*s= $(shell find %.*s -name '*.inc')\n", deps_first, "+", include->len, include->data);
				deps_first = 1;
			}

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

			if (lang & (1 << LANG_ASM)) {
				p_fprintf(fp, "DEPS %.*s= $(shell find %.*s -name '*.inc')\n", deps_first, "+", source->len, source->data);
				deps_first = 1;
			}

			if (lang & (1 << LANG_C) || lang & (1 << LANG_CPP)) {
				p_fprintf(fp, "DEPS %.*s= $(shell find %.*s -name '*.h')\n", deps_first, "+", source->len, source->data);
				deps_first = 1;
			}

			if (lang & (1 << LANG_CPP)) {
				p_fprintf(fp, "DEPS %.*s= $(shell find %.*s -name '*.hpp')\n", deps_first, "+", source->len, source->data);
				deps_first = 1;
			}

			if (lang & (1 << LANG_ASM)) {
				p_fprintf(fp, "SRC_ASM = $(shell find %.*s -name '*.asm')\n", source->len, source->data);
			}

			if (lang & (1 << LANG_C)) {
				p_fprintf(fp, "SRC_C = $(shell find %.*s -name '*.c')\n", source->len, source->data);
			}

			if (lang & (1 << LANG_CPP)) {
				p_fprintf(fp, "SRC_CPP = $(shell find %.*s -name '*.cpp')\n", source->len, source->data);
			}
		}
	}

	if (lang == (1 << LANG_ASM)) {
		p_fprintf(fp, "OBJ_ASM = $(patsubst %%.asm, $(INTDIR)%%.bin, $(SRC_ASM))\n");
	} else if (lang & (1 << LANG_ASM)) {
		p_fprintf(fp, "OBJ_ASM = $(patsubst %%.asm, $(INTDIR)%%.o, $(SRC_ASM))\n");
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
	case PROJ_TYPE_BIN:
		p_fprintf(fp, "TARGET = $(OUTDIR)%.*s.bin\n\n", name->len, name->data);
		break;
	case PROJ_TYPE_LIB:
	case PROJ_TYPE_EXT:
		p_fprintf(fp, "TARGET = $(OUTDIR)%.*s.a\n\n", name->len, name->data);
		break;
	default:
		p_fprintf(fp, "TARGET = $(OUTDIR)%.*s\n\n", name->len, name->data);
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

	if (lang & (1 << LANG_ASM)) {
		p_fprintf(fp, "ASMFLAGS =");

		if (lang == (1 << LANG_ASM)) {
			p_fprintf(fp, " -f bin");
		}

		//TODO: Remove
		if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
			const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;

			for (uint i = 0; i < sources->cnt; i++) {
				prop_str_t *source = arr_get(sources, i);

				buf_len = resolve(source, CSTR(buf), proj);
				p_fprintf(fp, " -i%.*s", buf_len, buf);
			}
		}

		if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
			const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;

			for (uint i = 0; i < includes->cnt; i++) {
				prop_str_t *include = arr_get(includes, i);

				buf_len = resolve(include, CSTR(buf), proj);
				p_fprintf(fp, " -i%.*s", buf_len, buf);
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
					p_fprintf(fp, " -i$(SLNDIR)/%.*s/%.*s", rel_path_len, rel_path, buf_len, buf);
				}
			}

			if (iproj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
				buf_len = resolve(&iproj->props[PROJ_PROP_ENCLUDE].value, CSTR(buf), iproj);
				p_fprintf(fp, " -i");
				print_rel_path(fp, iproj, buf, buf_len);
			}
		}

		p_fprintf(fp, "\n");
	}

	if (lang & (1 << LANG_C)) {
		p_fprintf(fp, "CFLAGS += $(FLAGS)");

		if ((cflags->flags & PROP_SET)) {
			if (cflags->mask & (1 << CFLAG_STD_C99)) {
				p_fprintf(fp, " -std=c99");
			}
			if (cflags->mask & (1 << CFLAG_FREESTANDING)) {
				p_fprintf(fp, " -ffreestanding");
			}
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

	if (lang & (1 << LANG_ASM)) {
		p_fprintf(fp, "ifeq (, $(shell which nasm))\n"
			      "\tsudo apt install nasm\n"
			      "endif\n");

		if (lang != (1 << LANG_ASM)) {
			p_fprintf(fp, "ifeq ($(PLATFORM), x86_64)\n"
				      "\t$(eval ASMFLAGS += -f elf64)\n"
				      "else\n"
				      "\t$(eval ASMFLAGS += -f elf32)\n"
				      "endif\n");
		}
	}

	if (lang & (1 << LANG_C) || lang & (1 << LANG_CPP)) {
		p_fprintf(fp, "ifeq (, $(shell which gcc))\n"
			      "\tsudo apt install gcc\n"
			      "endif\n");
	}

	if (requires->flags & PROP_SET) {
		for (uint i = 0; i < requires->arr.cnt; i++) {
			const prop_str_t *require = arr_get(&requires->arr, i);

			p_fprintf(fp,
				  "ifeq (, $(shell dpkg -l %.*s))\n"
				  "\tsudo apt install %.*s\n"
				  "endif\n",
				  require->len, require->data, require->len, require->data);
		}
	}

	p_fprintf(fp,
		  "\ncheck_coverage: check\n"
		  "\t$(eval CONFIG_FLAGS += --coverage -fprofile-abs-path)\n"
		  "ifeq (, $(shell which lcov))\n"
		  "\tsudo apt install lcov\n"
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

	bool dep_bin = 1;

	if (depends->flags & PROP_SET) {
		for (uint i = 0; i < depends->arr.cnt; i++) {
			prop_str_t *dpname = arr_get(&depends->arr, i);

			proj_t *dproj = NULL;
			if (hashmap_get(projects, dpname->data, dpname->len, (void **)&dproj)) {
				ERR("project doesn't exists: '%.*s'", (int)dpname->len, dpname->data);
				continue;
			}

			if (dproj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_BIN) {
				dep_bin = 0;
				break;
			}
		}

		if (dep_bin) {
			for (uint i = 0; i < depends->arr.cnt; i++) {
				const prop_str_t *dpname = arr_get(&depends->arr, i);

				proj_t *dproj = NULL;
				if (hashmap_get(projects, dpname->data, dpname->len, (void **)&dproj)) {
					ERR("project doesn't exists: '%.*s'", (int)dpname->len, dpname->data);
					continue;
				}

				const prop_t *doutdir = &dproj->props[PROJ_PROP_OUTDIR];

				buf_len = resolve(&doutdir->value, CSTR(buf), dproj);
				p_fprintf(fp, " ");
				print_rel_path(fp, dproj, buf, buf_len);

				p_fprintf(fp, "%.*s.bin", dproj->name->len, dproj->name->data);
			}
		}
	} else {
		dep_bin = 0;
	}

	if (lang & (1 << LANG_ASM)) {
		p_fprintf(fp, " $(OBJ_ASM)");
	}

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
	case PROJ_TYPE_BIN:
		if (lang == (1 << LANG_ASM)) {
			p_fprintf(fp, "\t@cp $^ $@\n");
		} else {
			p_fprintf(fp, "\t@$(CLD) -o $@ -Ttext 0x1000 $^ --oformat binary\n");
		}
		break;
	case PROJ_TYPE_EXE:
		if (dep_bin) {
			p_fprintf(fp, "\t@cat $^ > $@\n");
		} else {
			p_fprintf(fp, "\t@$(CCC) $(CONFIG_FLAGS) -o $@ $^ $(LDFLAGS)\n");
		}
		break;
	}

	p_fprintf(fp, "\n");

	if (lang == (1 << LANG_ASM)) {
		p_fprintf(fp, "$(INTDIR)%%.bin: %%.asm\n"
			      "\t@mkdir -p $(@D)\n"
			      "\t@nasm $< $(ASMFLAGS) -o $@\n\n");
	} else if (lang & (1 << LANG_ASM)) {
		p_fprintf(fp, "$(INTDIR)%%.o: %%.asm\n"
			      "\t@mkdir -p $(@D)\n"
			      "\t@nasm $< $(ASMFLAGS) -o $@\n\n");
	}

	if (lang & (1 << LANG_C)) {
		p_fprintf(fp, "$(INTDIR)%%.o: %%.c\n"
			      "\t@mkdir -p $(@D)\n"
			      "\t@$(CCC) $(CONFIG_FLAGS) $(CFLAGS) -c -o $@ $<\n\n");
	}

	if (lang & (1 << LANG_CPP)) {
		p_fprintf(fp, "$(INTDIR)%%.o: %%.cpp\n"
			      "\t@mkdir -p $(@D)\n"
			      "\t@$(CCC) $(CONFIG_FLAGS) $(CXXFLAGS) -c -o $@ $<\n\n");
	}

	p_fprintf(fp, "clean:\n"
		      "\t@$(RM) $(TARGET)");

	if (lang & (1 << LANG_ASM)) {
		p_fprintf(fp, " $(OBJ_ASM)");
	}

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
}

static int gen_url(const proj_t *proj, FILE *fp)
{
	const prop_str_t *name	 = proj->name;
	const prop_str_t *url	 = &proj->props[PROJ_PROP_URL].value;
	const prop_str_t *format = &proj->props[PROJ_PROP_FORMAT].value;
	const prop_str_t *config = &proj->props[PROJ_PROP_CONFIG].value;
	const prop_str_t *target = &proj->props[PROJ_PROP_TARGET].value;
	const prop_t *requires	 = &proj->props[PROJ_PROP_REQUIRE];

	p_fprintf(fp,
		  "URL = %.*s\n"
		  "NAME = %.*s\n"
		  "FORMAT = %.*s\n"
		  "FILE = $(NAME).$(FORMAT)\n"
		  "DLDIR = $(SLNDIR)/dl/$(FILE)\n"
		  "SRCDIR = $(SLNDIR)/staging/$(NAME)\n"
		  "BUILDDIR = $(SLNDIR)/build/$(PLATFORM)/$(NAME)\n"
		  "LOGDIR = $(SLNDIR)/logs/$(PLATFORM)/$(NAME)\n"
		  "\n"
		  ".PHONY: all check compile clean\n"
		  "\n"
		  "check:\n"
		  "ifeq (, $(shell which curl))\n"
		  "\tsudo apt install curl\n"
		  "endif\n",
		  url->len, url->data, name->len, name->data, format->len, format->data);

	if (requires->flags & PROP_SET) {
		for (uint i = 0; i < requires->arr.cnt; i++) {
			const prop_str_t *require = arr_get(&requires->arr, i);

			p_fprintf(fp,
				  "ifeq (, $(shell dpkg -l %.*s))\n"
				  "\tsudo apt install %.*s\n"
				  "endif\n",
				  require->len, require->data, require->len, require->data);
		}
	}

	p_fprintf(fp,
		  "\n"
		  "all: compile\n"
		  "\n"
		  "$(DLDIR):\n"
		  "	@mkdir -p $(@D)\n"
		  "	@cd $(@D) && curl -O $(URL)$(FILE)\n"
		  "\n"
		  "$(SRCDIR)/done: $(DLDIR)\n"
		  "	@mkdir -p $(@D)\n"
		  "	@tar xf $(DLDIR) -C $(SRCDIR)\n"
		  "	@touch $(SRCDIR)/done\n"
		  "\n"
		  "$(OUTDIR)/$(NAME): $(SRCDIR)/done\n"
		  "	@mkdir -p $(LOGDIR) $(BUILDDIR) $(OUTDIR)\n"
		  "	@cd $(BUILDDIR) && $(SRCDIR)/configure --target=$(PLATFORM)-elf --prefix=$(OUTDIR) %.*s 2>&1 | tee $(LOGDIR)/configure.log\n"
		  "	@cd $(BUILDDIR) && make %.*s 2>&1 | tee $(LOGDIR)/make.log\n"
		  "	@touch $(OUTDIR)/$(NAME)\n"
		  "\n"
		  "compile: check $(OUTDIR)/$(NAME)\n"
		  "\n"
		  "clean:\n",
		  config->len, config->data, target->len, target->data);
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
	const prop_t *url     = &proj->props[PROJ_PROP_URL];

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

	if (outdir->flags & PROP_SET) {
		buf_len = resolve(&outdir->value, CSTR(buf), proj);
		p_fprintf(fp, "OUTDIR = %.*s\n", buf_len, buf);
	}

	if (url->flags & PROP_SET) {
		gen_url(proj, fp);
	} else {
		gen_source(proj, projects, sln_props, fp);
	}

	file_close(fp);
	if (ret == 0) {
		SUC("generating project: %s success", cmake_path.path);
	} else {
		ERR("generating project: %s failed", cmake_path.path);
	}

	return ret;
}
