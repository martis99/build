#include "mk_proj.h"

#include "gen/sln.h"
#include "gen/var.h"

#include "common.h"

#include "make.h"

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

static size_t resolve(const prop_str_t *prop, char *buf, size_t buf_size, const proj_t *proj)
{
	size_t buf_len = prop->len;
	mem_cpy(buf, buf_size, prop->data, prop->len);

	buf_len = invert_slash(buf, buf_len);
	buf_len = cstr_replaces(buf, buf_size, buf_len, vars.old, vars.new, __VAR_MAX, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_NAME)"), proj->name->data, proj->name->len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_FOLDER)"), proj->rel_path.path, proj->rel_path.len, NULL);

	return buf_len;
}

static inline void add_rel_path(make_t *make, make_var_t var, const proj_t *proj, const char *prefix, const char *path, size_t path_len)
{
	char path_b[P_MAX_PATH]	  = { 0 };
	size_t path_b_len	  = convert_slash(CSTR(path_b), path, path_len);
	char rel_path[P_MAX_PATH] = { 0 };
	size_t rel_path_len	  = convert_slash(CSTR(rel_path), proj->rel_path.path, proj->rel_path.len);

	if (cstr_eqn(path_b, path_b_len, CSTR("$(SLNDIR)"), 9)) {
		make_var_add_val(make, var, MSTR(strf("%s%.*s", prefix, path_b_len, path_b)));
	} else {
		make_var_add_val(make, var, MSTR(strf("%s$(SLNDIR)/%.*s/%.*s", prefix, rel_path_len, rel_path, path_b_len, path_b)));
	}
}

static int gen_source(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, make_t *make, make_var_t mplatform, make_var_t mconfig)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *configs	= &sln_props[SLN_PROP_CONFIGS];
	const prop_t *langs	= &proj->props[PROJ_PROP_LANGS];
	const prop_t *cflags	= &proj->props[PROJ_PROP_CFLAGS];
	const prop_t *ccflags	= &proj->props[PROJ_PROP_CCFLAGS];
	const prop_t *outdir	= &proj->props[PROJ_PROP_OUTDIR];
	const prop_t *intdir	= &proj->props[PROJ_PROP_INTDIR];
	const prop_t *depends	= &proj->props[PROJ_PROP_DEPENDS];
	const prop_t *files	= &proj->props[PROJ_PROP_FILES];
	const prop_t *requires	= &proj->props[PROJ_PROP_REQUIRE];
	const prop_t *run	= &proj->props[PROJ_PROP_RUN];
	const prop_t *drun	= &proj->props[PROJ_PROP_DRUN];
	const prop_t *size	= &proj->props[PROJ_PROP_SIZE];
	const prop_t *pfilename = &proj->props[PROJ_PROP_FILENAME];
	const prop_t *partifact = &proj->props[PROJ_PROP_ARTIFACT];

	const prop_str_t *filename = name;
	if (pfilename->flags & PROP_SET) {
		filename = &pfilename->value;
	}

	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len;

	uint lang = langs->mask;

	make_var_type_t deps_type = MAKE_VAR_INST;

	int ret = 0;

	if (intdir->flags & PROP_SET) {
		buf_len			 = resolve(&intdir->value, CSTR(buf), proj);
		const make_var_t mintdir = make_add_act(make, make_create_var(make, STR("INTDIR"), MAKE_VAR_INST));
		make_var_add_val(make, mintdir, MSTR(strn(buf, buf_len, buf_len + 1)));
	}

	make_var_t repdir = MAKE_END;
	if (type == PROJ_TYPE_EXE) {
		repdir = make_add_act(make, make_create_var(make, STR("REPDIR"), MAKE_VAR_INST));
		make_var_add_val(make, repdir, MSTR(STR("$(OUTDIR)coverage-report/")));
	}

	if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
		const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;

		for (uint i = 0; i < includes->cnt; i++) {
			const prop_str_t *include = arr_get(includes, i);

			if (lang & (1 << LANG_ASM)) {
				make_var_t deps = make_add_act(make, make_create_var(make, STR("DEPS"), deps_type));
				make_var_add_val(make, deps, MSTR(strf("$(shell find %.*s -name '*.inc')", include->len, include->data)));
				deps_type = MAKE_VAR_APP;
			}

			if (lang & (1 << LANG_C) || lang & (1 << LANG_CPP)) {
				make_var_t deps = make_add_act(make, make_create_var(make, STR("DEPS"), deps_type));
				make_var_add_val(make, deps, MSTR(strf("$(shell find %.*s -name '*.h')", include->len, include->data)));
				deps_type = MAKE_VAR_APP;
			}

			if (lang & (1 << LANG_CPP)) {
				make_var_t deps = make_add_act(make, make_create_var(make, STR("DEPS"), deps_type));
				make_var_add_val(make, deps, MSTR(strf("$(shell find %.*s -name '*.hpp')", include->len, include->data)));
				deps_type = MAKE_VAR_APP;
			}
		}
	}

	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;

		for (uint i = 0; i < sources->cnt; i++) {
			const prop_str_t *source = arr_get(sources, i);

			if (lang & (1 << LANG_ASM)) {
				const make_var_t deps = make_add_act(make, make_create_var(make, STR("DEPS"), deps_type));
				make_var_add_val(make, deps, MSTR(strf("$(shell find %.*s -name '*.inc')", source->len, source->data)));
				deps_type = MAKE_VAR_APP;
			}

			if (lang & (1 << LANG_C) || lang & (1 << LANG_CPP)) {
				const make_var_t deps = make_add_act(make, make_create_var(make, STR("DEPS"), deps_type));
				make_var_add_val(make, deps, MSTR(strf("$(shell find %.*s -name '*.h')", source->len, source->data)));
				deps_type = MAKE_VAR_APP;
			}

			if (lang & (1 << LANG_CPP)) {
				const make_var_t deps = make_add_act(make, make_create_var(make, STR("DEPS"), deps_type));
				make_var_add_val(make, deps, MSTR(strf("$(shell find %.*s -name '*.hpp')", source->len, source->data)));
				deps_type = MAKE_VAR_APP;
			}

			if (lang & (1 << LANG_ASM)) {
				const make_var_t src_asm = make_add_act(make, make_create_var(make, STR("SRC_ASM"), MAKE_VAR_INST));
				make_var_add_val(make, src_asm, MSTR(strf("$(shell find %.*s -name '*.asm')", source->len, source->data)));
			}

			if (lang & (1 << LANG_C)) {
				const make_var_t src_c = make_add_act(make, make_create_var(make, STR("SRC_C"), MAKE_VAR_INST));
				make_var_add_val(make, src_c, MSTR(strf("$(shell find %.*s -name '*.c')", source->len, source->data)));
			}

			if (lang & (1 << LANG_CPP)) {
				const make_var_t src_cpp = make_add_act(make, make_create_var(make, STR("SRC_CPP"), MAKE_VAR_INST));
				make_var_add_val(make, src_cpp, MSTR(strf("$(shell find %.*s -name '*.cpp')", source->len, source->data)));
			}
		}
	}

	make_var_t obj_asm = MAKE_END;
	if (lang == (1 << LANG_ASM)) {
		obj_asm = make_add_act(make, make_create_var(make, STR("OBJ_ASM"), MAKE_VAR_INST));
		make_var_add_val(make, obj_asm, MSTR(STR("$(patsubst %.asm, $(INTDIR)%.bin, $(SRC_ASM))")));
	} else if (lang & (1 << LANG_ASM)) {
		obj_asm = make_add_act(make, make_create_var(make, STR("OBJ_ASM"), MAKE_VAR_INST));
		make_var_add_val(make, obj_asm, MSTR(STR("$(patsubst %.asm, $(INTDIR)%.o, $(SRC_ASM))")));
	}

	make_var_t obj_c = MAKE_END;
	if (lang & (1 << LANG_C)) {
		obj_c = make_add_act(make, make_create_var(make, STR("OBJ_C"), MAKE_VAR_INST));
		make_var_add_val(make, obj_c, MSTR(STR("$(patsubst %.c, $(INTDIR)%.o, $(SRC_C))")));
	}

	make_var_t obj_cpp = MAKE_END;
	if (lang & (1 << LANG_CPP)) {
		obj_cpp = make_add_act(make, make_create_var(make, STR("OBJ_CPP"), MAKE_VAR_INST));
		make_var_add_val(make, obj_cpp, MSTR(STR("$(patsubst %.cpp, $(INTDIR)%.o, $(SRC_CPP))")));
	}

	if (proj_coverable(proj)) {
		const make_var_t lcov = make_add_act(make, make_create_var(make, STR("LCOV"), MAKE_VAR_INST));
		make_var_add_val(make, lcov, MSTR(STR("$(OUTDIR)lcov.info")));

		bool cov_first		 = 1;
		make_var_type_t cov_type = MAKE_VAR_INST;

		if (lang & (1 << LANG_C)) {
			make_var_t cov = make_add_act(make, make_create_var(make, STR("COV"), cov_type));
			make_var_add_val(make, cov, MSTR(STR("$(patsubst %.c, $(INTDIR)%.gcno, $(SRC_C))")));
			cov_type = MAKE_VAR_APP;
			cov	 = make_add_act(make, make_create_var(make, STR("COV"), cov_type));
			make_var_add_val(make, cov, MSTR(STR("$(patsubst %.c, $(INTDIR)%.gcda, $(SRC_C))")));
			cov_type = MAKE_VAR_APP;
		}

		if (lang & (1 << LANG_CPP)) {
			make_var_t cov = make_add_act(make, make_create_var(make, STR("COV"), cov_type));
			make_var_add_val(make, cov, MSTR(STR("$(patsubst %.cpp, $(INTDIR)%.gcno, $(SRC_CPP))")));
			cov_type = MAKE_VAR_APP;
			cov	 = make_add_act(make, make_create_var(make, STR("COV"), cov_type));
			make_var_add_val(make, cov, MSTR(STR("$(patsubst %.cpp, $(INTDIR)%.gcda, $(SRC_CPP))")));
			cov_type = MAKE_VAR_APP;
		}

		const make_var_t cov = make_add_act(make, make_create_var(make, STR("COV"), cov_type));
		make_var_add_val(make, cov, MVAR(lcov));
		if (repdir != MAKE_END) {
			make_var_add_val(make, cov, MVAR(repdir));
		}
		cov_type = MAKE_VAR_APP;
	}

	const make_var_t target = make_add_act(make, make_create_var(make, STR("TARGET"), MAKE_VAR_INST));

	switch (type) {
	case PROJ_TYPE_EXE:
		make_var_add_val(make, target, MSTR(strf("$(OUTDIR)%.*s", filename->len, filename->data)));
		break;
	case PROJ_TYPE_BIN:
		make_var_add_val(make, target, MSTR(strf("$(OUTDIR)%.*s.bin", filename->len, filename->data)));
		break;
	case PROJ_TYPE_ELF:
		make_var_add_val(make, target, MSTR(strf("$(OUTDIR)%.*s.elf", filename->len, filename->data)));
		break;
	case PROJ_TYPE_FAT12:
		make_var_add_val(make, target, MSTR(strf("$(OUTDIR)%.*s.img", filename->len, filename->data)));
		break;
	case PROJ_TYPE_LIB:
	case PROJ_TYPE_EXT:
		make_var_add_val(make, target, MSTR(strf("$(OUTDIR)%.*s.a", filename->len, filename->data)));
		break;
	}

	make_var_t martifact = MAKE_END;
	if (partifact->flags & PROP_SET) {
		const make_var_t martifactdir = make_add_act(make, make_create_var(make, STR("ARTIFACTDIR"), MAKE_VAR_INST));
		make_var_add_val(make, martifactdir, MSTR(STR("$(SLNDIR)/tmp/artifact/")));
		martifact = make_add_act(make, make_create_var(make, STR("ARTIFACT"), MAKE_VAR_INST));
		make_var_add_val(make, martifact, MSTR(strf("$(ARTIFACTDIR)%.*s", partifact->value.len, partifact->value.data)));
	}

	make_add_act(make, make_create_empty(make));

	const make_if_t if_platform = make_add_act(make, make_create_if(make, MVAR(mplatform), MSTR(STR("x86_64"))));
	const make_var_t bits_64    = make_if_add_true_act(make, if_platform, make_create_var(make, STR("BITS"), MAKE_VAR_INST));
	make_var_add_val(make, bits_64, MSTR(STR("64")));
	const make_var_t bits_32 = make_if_add_false_act(make, if_platform, make_create_var(make, STR("BITS"), MAKE_VAR_INST));
	make_var_add_val(make, bits_32, MSTR(STR("32")));

	make_add_act(make, make_create_empty(make));

	const make_var_t mflags = make_add_act(make, make_create_var(make, STR("FLAGS"), MAKE_VAR_INST));

	//TODO: Remove
	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;

		for (uint i = 0; i < sources->cnt; i++) {
			const prop_str_t *source = arr_get(sources, i);

			buf_len = resolve(source, CSTR(buf), proj);
			make_var_add_val(make, mflags, MSTR(strf("-I%.*s", buf_len, buf)));
		}
	}

	if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
		const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;

		for (uint i = 0; i < includes->cnt; i++) {
			const prop_str_t *include = arr_get(includes, i);

			buf_len = resolve(include, CSTR(buf), proj);
			make_var_add_val(make, mflags, MSTR(strf("-I%.*s", buf_len, buf)));
		}
	}

	for (uint i = 0; i < proj->includes.cnt; i++) {
		const proj_t *iproj = *(proj_t **)arr_get(&proj->includes, i);

		if (iproj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
			const arr_t *includes = &iproj->props[PROJ_PROP_INCLUDE].arr;

			for (uint i = 0; i < includes->cnt; i++) {
				const prop_str_t *include = arr_get(includes, i);

				char rel_path[P_MAX_PATH] = { 0 };
				size_t rel_path_len;

				buf_len	     = resolve(include, CSTR(buf), iproj);
				rel_path_len = convert_slash(CSTR(rel_path), iproj->rel_path.path, iproj->rel_path.len);
				make_var_add_val(make, mflags, MSTR(strf("-I$(SLNDIR)/%.*s/%.*s", rel_path_len, rel_path, buf_len, buf)));
			}
		}

		if (iproj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
			buf_len = resolve(&iproj->props[PROJ_PROP_ENCLUDE].value, CSTR(buf), iproj);
			add_rel_path(make, mflags, iproj, "-I", buf, buf_len);
		}
	}

	if (proj->props[PROJ_PROP_DEFINES].flags & PROP_SET) {
		const arr_t *defines = &proj->props[PROJ_PROP_DEFINES].arr;

		for (uint k = 0; k < defines->cnt; k++) {
			const prop_str_t *define = arr_get(defines, k);

			make_var_add_val(make, mflags, MSTR(strf("-D%.*s", define->len, define->data)));
		}
	}

	if (lang & (1 << LANG_ASM)) {
		const make_var_t asmflags = make_add_act(make, make_create_var(make, STR("ASMFLAGS"), MAKE_VAR_APP));
		make_var_add_val(make, asmflags, MVAR(mflags));

		if (lang == (1 << LANG_ASM)) {
			make_var_add_val(make, asmflags, MSTR(STR("-fbin")));
		} else {
			make_var_add_val(make, asmflags, MSTR(STR("-felf$(BITS)")));
		}
	}

	if (lang & (1 << LANG_C)) {
		const make_var_t mcflags = make_add_act(make, make_create_var(make, STR("CFLAGS"), MAKE_VAR_APP));
		make_var_add_val(make, mcflags, MVAR(mflags));

		if ((cflags->flags & PROP_SET)) {
			if (cflags->mask & (1 << CFLAG_STD_C99)) {
				make_var_add_val(make, mcflags, MSTR(STR("-std=c99")));
			}
			if (cflags->mask & (1 << CFLAG_FREESTANDING)) {
				make_var_add_val(make, mcflags, MSTR(STR("-ffreestanding")));
			}
		}

		if (ccflags->flags & PROP_SET) {
			for (uint i = 0; i < ccflags->arr.cnt; i++) {
				const prop_str_t *ccflag = arr_get(&ccflags->arr, i);
				make_var_add_val(make, mcflags, MSTR(strc(ccflag->data, ccflag->len)));
			}
		}
	}

	if (lang & (1 << LANG_CPP)) {
		const make_var_t cxxflags = make_add_act(make, make_create_var(make, STR("CXXFLAGS"), MAKE_VAR_APP));
		make_var_add_val(make, cxxflags, MVAR(mflags));
	}

	bool dep_bin = 1;

	if (depends->flags & PROP_SET) {
		for (uint i = 0; i < depends->arr.cnt; i++) {
			const prop_str_t *dpname = arr_get(&depends->arr, i);

			const proj_t *dproj = NULL;
			if (dict_get(projects, dpname->data, dpname->len, (void **)&dproj)) {
				ERR("project doesn't exists: '%.*s'", (int)dpname->len, dpname->data);
				continue;
			}

			if (dproj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_BIN) {
				dep_bin = 0;
				break;
			}
		}
	} else {
		dep_bin = 0;
	}

	if (!dep_bin && (lang & (1 << LANG_C) || lang & (1 << LANG_CPP)) && (type == PROJ_TYPE_EXE || type == PROJ_TYPE_BIN || type == PROJ_TYPE_ELF)) {
		const make_var_t mldflags = make_add_act(make, make_create_var(make, STR("LDFLAGS"), MAKE_VAR_APP));

		const prop_t *ldflags = &proj->props[PROJ_PROP_LDFLAGS];

		if (ldflags->flags & PROP_SET) {
			if (ldflags->mask & (1 << LDFLAG_WHOLEARCHIVE)) {
				make_var_add_val(make, mldflags, MSTR(STR("-Wl,--whole-archive")));
			}
			if (ldflags->mask & (1 << LDFLAG_ALLOWMULTIPLEDEFINITION)) {
				make_var_add_val(make, mldflags, MSTR(STR("-Wl,--allow-multiple-definition")));
			}
		}

		for (int i = proj->all_depends.cnt - 1; i >= 0; i--) {
			const proj_t *dproj = *(proj_t **)arr_get(&proj->all_depends, i);

			if (dproj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
				const prop_t *doutdir = &dproj->props[PROJ_PROP_OUTDIR];

				if (doutdir->flags & PROP_SET) {
					buf_len = resolve(&doutdir->value, CSTR(buf), dproj);
					add_rel_path(make, mldflags, dproj, "-L", buf, buf_len);

					make_var_add_val(make, mldflags, MSTR(strf("-l:%.*s.a", dproj->name->len, dproj->name->data)));
				} else {
					buf_len = convert_slash(CSTR(buf), dproj->rel_path.path, dproj->rel_path.len);
					make_var_add_val(make, mldflags, MSTR(strf("-L$(SLNDIR)/%.*s -l:%.*s.a", buf_len, buf, dproj->name->len, dproj->name->data)));
				}
			}

			if (dproj->props[PROJ_PROP_LIBDIRS].flags & PROP_SET) {
				const arr_t *libdirs = &dproj->props[PROJ_PROP_LIBDIRS].arr;
				for (uint j = 0; j < libdirs->cnt; j++) {
					const prop_str_t *libdir = arr_get(libdirs, j);
					if (libdir->len > 0) {
						buf_len = resolve(libdir, CSTR(buf), dproj);
						add_rel_path(make, mldflags, dproj, "-L", buf, buf_len);
					}
				}
			}

			if (dproj->props[PROJ_PROP_LINK].flags & PROP_SET) {
				const arr_t *links = &dproj->props[PROJ_PROP_LINK].arr;
				for (uint j = 0; j < links->cnt; j++) {
					const prop_str_t *link = arr_get(links, j);
					if (link->len > 0) {
						make_var_add_val(make, mldflags, MSTR(strf(" -l:%.*s.a", link->len, link->data)));
					}
				}
			}
		}

		if ((ldflags->flags & PROP_SET) && (ldflags->mask & (1 << LDFLAG_WHOLEARCHIVE))) {
			make_var_add_val(make, mldflags, MSTR(STR("-Wl,--no-whole-archive")));
		}

		if (lang & (1 << LANG_CPP)) {
			make_var_add_val(make, mldflags, MSTR(STR("-lstdc++")));
		}

		if (ldflags->flags & PROP_SET) {
			if (ldflags->mask & (1 << LDFLAG_MATH)) {
				make_var_add_val(make, mldflags, MSTR(STR("-lm")));
			}

			if (ldflags->mask & (1 << LDFLAG_X11)) {
				make_var_add_val(make, mldflags, MSTR(STR("-lX11")));
			}

			if (ldflags->mask & (1 << LDFLAG_GL)) {
				make_var_add_val(make, mldflags, MSTR(STR("-lGL")));
			}

			if (ldflags->mask & (1 << LDFLAG_GLX)) {
				make_var_add_val(make, mldflags, MSTR(STR("-lGLX")));
			}
		}
	}

	make_add_act(make, make_create_empty(make));

	const make_var_t rm = make_add_act(make, make_create_var(make, STR("RM"), MAKE_VAR_APP));
	make_var_add_val(make, rm, MSTR(STR("-r")));

	make_add_act(make, make_create_empty(make));
	const make_var_t config_flags = make_add_act(make, make_create_var(make, STR("CONFIG_FLAGS"), MAKE_VAR_INST));

	make_add_act(make, make_create_empty(make));

	if ((configs->flags & PROP_SET) && configs->arr.cnt > 0) {
		const make_if_t if_config	= make_add_act(make, make_create_if(make, MVAR(mconfig), MSTR(STR("Debug"))));
		const make_var_t config_flags_a = make_if_add_true_act(make, if_config, make_create_var(make, STR("CONFIG_FLAGS"), MAKE_VAR_APP));
		make_var_add_val(make, config_flags_a, MSTR(STR("-ggdb3 -O0")));

		make_add_act(make, make_create_empty(make));
	}

	make_var_t show = MAKE_END;

	if (type == PROJ_TYPE_EXE) {
		show = make_add_act(make, make_create_var(make, STR("SHOW"), MAKE_VAR_INST));
		make_var_add_val(make, show, MSTR(STR("true")));

		make_add_act(make, make_create_empty(make));
	}

	const make_rule_t all = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("all"))), 0));
	make_rule_add_depend(make, all, MRULE(MSTR(strc(name->data, name->len))));

	const make_rule_t check = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("check"))), 0));

	if (lang & (1 << LANG_ASM)) {
		const make_if_t if_nasm = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which nasm)"))));
		make_if_add_true_act(make, if_nasm, make_create_cmd(make, MCMD(STR("sudo apt install nasm"))));
	}

	if (lang & (1 << LANG_C) || lang & (1 << LANG_CPP)) {
		const make_if_t if_gcc = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which gcc)"))));
		make_if_add_true_act(make, if_gcc, make_create_cmd(make, MCMD(STR("sudo apt install gcc"))));
	}

	if (type == PROJ_TYPE_FAT12) {
		const make_if_t if_mcopy = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which mcopy)"))));
		make_if_add_true_act(make, if_mcopy, make_create_cmd(make, MCMD(STR("sudo apt install mtools"))));
	}

	if (requires->flags & PROP_SET) {
		for (uint i = 0; i < requires->arr.cnt; i++) {
			const prop_str_t *require = arr_get(&requires->arr, i);

			const make_if_t if_dpkg =
				make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(strf("$(shell dpkg -l %.*s)", require->len, require->data))));
			make_if_add_true_act(make, if_dpkg, make_create_cmd(make, MCMD(strf("sudo apt install %.*s", require->len, require->data))));
		}
	}

	if (proj_coverable(proj)) {
		const make_rule_t check_coverage = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("check_coverage"))), 0));
		make_rule_add_depend(make, check_coverage, MRULE(MSTR(STR("check"))));
		make_rule_add_act(make, check_coverage, make_create_cmd(make, MCMD(STR("$(eval CONFIG_FLAGS += --coverage -fprofile-abs-path)"))));
		if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE) {
			const make_if_t if_lcov = make_rule_add_act(make, check_coverage, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which lcov)"))));
			make_if_add_true_act(make, if_lcov, make_create_cmd(make, MCMD(STR("sudo apt install lcov"))));
		}
	}

	const make_rule_t mname = make_add_act(make, make_create_rule(make, MRULE(MSTR(strc(name->data, name->len))), 0));
	make_rule_add_depend(make, mname, MRULE(MSTR(STR("clean"))));
	make_rule_add_depend(make, mname, MRULE(MSTR(STR("compile"))));

	const make_rule_t compile = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("compile"))), 0));
	make_rule_add_depend(make, compile, MRULE(MSTR(STR("check"))));
	make_rule_add_depend(make, compile, MRULE(MVAR(target)));

	if (proj_coverable(proj)) {
		const make_rule_t coverage = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("coverage"))), 0));
		make_rule_add_depend(make, coverage, MRULE(MSTR(STR("clean"))));
		make_rule_add_depend(make, coverage, MRULE(MSTR(STR("check_coverage"))));
		make_rule_add_depend(make, coverage, MRULE(MVAR(target)));

		if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE) {
			make_rule_add_act(make, coverage, make_create_cmd(make, MCMD(STR("@$(TARGET) $(ARGS)"))));
			make_rule_add_act(make, coverage, make_create_cmd(make, MCMD(STR("@lcov -q -c -d $(SLNDIR) -o $(LCOV)"))));
			const make_if_t if_show = make_rule_add_act(make, coverage, make_create_if(make, MVAR(show), MSTR(STR("true"))));
			make_if_add_true_act(make, if_show, make_create_cmd(make, MCMD(STR("@genhtml -q $(LCOV) -o $(REPDIR)"))));
			make_if_add_true_act(make, if_show, make_create_cmd(make, MCMD(STR("@open $(REPDIR)index.html"))));
		}
	}

	const make_rule_t rtarget = make_add_act(make, make_create_rule(make, MRULE(MVAR(target)), 1));

	if (dep_bin) {
		for (uint i = 0; i < depends->arr.cnt; i++) {
			const prop_str_t *dpname = arr_get(&depends->arr, i);

			const proj_t *dproj = NULL;
			if (dict_get(projects, dpname->data, dpname->len, (void **)&dproj)) {
				ERR("project doesn't exists: '%.*s'", (int)dpname->len, dpname->data);
				continue;
			}

			const prop_t *doutdir = &dproj->props[PROJ_PROP_OUTDIR];

			buf_len			  = resolve(&doutdir->value, CSTR(buf), dproj);
			char path_b[P_MAX_PATH]	  = { 0 };
			size_t path_b_len	  = convert_slash(CSTR(path_b), buf, buf_len);
			char rel_path[P_MAX_PATH] = { 0 };
			size_t rel_path_len	  = convert_slash(CSTR(rel_path), proj->rel_path.path, proj->rel_path.len);

			if (cstr_eqn(path_b, path_b_len, CSTR("$(SLNDIR)"), 9)) {
				make_rule_add_depend(make, rtarget, MRULE(MSTR(strf("%.*s%.*s.bin", path_b_len, path_b, dproj->name->len, dproj->name->data))));
			} else {
				make_rule_add_depend(make, rtarget,
						     MRULE(MSTR(strf("$(SLNDIR)/%.*s/%.*s%.*s.bin", rel_path_len, rel_path, path_b_len, path_b, dproj->name->len,
								     dproj->name->data))));
			}
		}
	}

	if (lang & (1 << LANG_ASM)) {
		make_rule_add_depend(make, rtarget, MRULE(MVAR(obj_asm)));
	}

	if (lang & (1 << LANG_C)) {
		make_rule_add_depend(make, rtarget, MRULE(MVAR(obj_c)));
	}

	if (lang & (1 << LANG_CPP)) {
		make_rule_add_depend(make, rtarget, MRULE(MVAR(obj_cpp)));
	}

	make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));

	switch (type) {
	case PROJ_TYPE_LIB:
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@ar rcs $@ $^"))));
		break;
	case PROJ_TYPE_BIN:
		if (files->flags & PROP_SET) {
			for (uint i = 0; i < files->arr.cnt; i++) {
				const prop_str_t *fname = arr_get(&files->arr, i);

				proj_t *fproj = NULL;
				if (dict_get(projects, fname->data, fname->len, (void **)&fproj)) {
					ERR("project doesn't exists: '%.*s'", (int)fname->len, fname->data);
					continue;
				}

				make_expand(&fproj->make);
				str_t mtarget = make_var_get_expanded(&fproj->make, STR("TARGET"));

				switch (fproj->props[PROJ_PROP_TYPE].mask) {
				case PROJ_TYPE_BIN:
					make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(strf("@dd if=%.*s status=none >> $@", mtarget.len, mtarget.data))));
					break;

				case PROJ_TYPE_ELF:
					make_rule_add_act(make, rtarget,
							  make_create_cmd(make, MCMD(strf("@objcopy -O binary -j .text %.*s %.*s.bin", mtarget.len, mtarget.data,
											  mtarget.len, mtarget.data))));
					make_rule_add_act(make, rtarget,
							  make_create_cmd(make, MCMD(strf("@dd if=%.*s.bin status=none >> $@", mtarget.len, mtarget.data))));
					break;
				}
			}

			if (size->flags & PROP_SET) {
				make_rule_add_act(make, rtarget,
						  make_create_cmd(make,
								  MCMD(strf("@dd if=/dev/zero bs=1 count=%.*s status=none >> $@", size->value.len, size->value.data))));
			}
		} else if (lang == (1 << LANG_ASM) || dep_bin) {
			make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@cat $^ > $@"))));
		} else {
			make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@$(TLD) -o $@ -Tlinker.ld $^ --oformat binary $(LDFLAGS)"))));
		}

		break;
	case PROJ_TYPE_ELF:
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@$(TLD) -o $@ -Tlinker.ld $^ $(LDFLAGS)"))));
		break;
	case PROJ_TYPE_EXE:
		if (dep_bin) {
			make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@cat $^ > $@"))));
		} else {
			make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@$(TCC) $(CONFIG_FLAGS) -o $@ $^ $(LDFLAGS)"))));
		}
		break;
	case PROJ_TYPE_FAT12:
		for (uint i = 0; i < files->arr.cnt; i++) {
			const prop_str_t *fname = arr_get(&files->arr, i);

			proj_t *fproj = NULL;
			if (dict_get(projects, fname->data, fname->len, (void **)&fproj)) {
				ERR("project doesn't exists: '%.*s'", (int)fname->len, fname->data);
				continue;
			}

			make_expand(&fproj->make);
			str_t mtarget = make_var_get_expanded(&fproj->make, STR("TARGET"));

			make_rule_add_depend(make, rtarget, MRULE(MSTR(str_cpy(mtarget))));
		}
		//create empty 1.44MB image (block size = 512, block count = 2880)
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@dd if=/dev/zero of=$@ bs=512 count=2880 status=none"))));
		//create file system
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@mkfs.fat -F12 -n \"NBOS\" $@"))));
		//put first binary to the first sector of the disk
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@dd if=$< of=$@ conv=notrunc status=none"))));
		//copy files to the image
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@mcopy -i $@ $(word 2,$^) \"::$(shell basename $(word 2,$^))\""))));
		break;
	}

	if (lang == (1 << LANG_ASM)) {
		const make_rule_t int_bin = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("$(INTDIR)%.bin"))), 1));
		make_rule_add_depend(make, int_bin, MRULE(MSTR(STR("%.asm"))));
		make_rule_add_act(make, int_bin, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, int_bin, make_create_cmd(make, MCMD(STR("@nasm $< $(ASMFLAGS) -o $@"))));
	} else if (lang & (1 << LANG_ASM)) {
		const make_rule_t int_o = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("$(INTDIR)%.o"))), 1));
		make_rule_add_depend(make, int_o, MRULE(MSTR(STR("%.asm"))));
		make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@nasm $< $(ASMFLAGS) -o $@"))));
	}

	if (lang & (1 << LANG_C)) {
		const make_rule_t int_o = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("$(INTDIR)%.o"))), 1));
		make_rule_add_depend(make, int_o, MRULE(MSTR(STR("%.c"))));
		make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@$(TCC) $(CONFIG_FLAGS) $(CFLAGS) -c -o $@ $<"))));
	}

	if (lang & (1 << LANG_CPP)) {
		const make_rule_t int_o = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("$(INTDIR)%.o"))), 1));
		make_rule_add_depend(make, int_o, MRULE(MSTR(STR("%.cpp"))));
		make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@$(TCC) $(CONFIG_FLAGS) $(CXXFLAGS) -c -o $@ $<"))));
	}

	if (proj_runnable(proj)) {
		const make_rule_t mrun = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("run"))), 0));
		make_rule_add_depend(make, mrun, MRULE(MSTR(STR("check"))));
		make_rule_add_depend(make, mrun, MRULE(MVAR(target)));

		if (run->flags & PROP_SET && drun->flags & PROP_SET) {
			const make_if_t if_config = make_rule_add_act(make, mrun, make_create_if(make, MVAR(mconfig), MSTR(STR("Debug"))));
			make_if_add_true_act(make, if_config, make_create_cmd(make, MCMD(strc(drun->value.data, drun->value.len))));
			make_if_add_false_act(make, if_config, make_create_cmd(make, MCMD(strc(run->value.data, run->value.len))));

		} else if (run->flags & PROP_SET) {
			make_rule_add_act(make, mrun, make_create_cmd(make, MCMD(strc(run->value.data, run->value.len))));
		} else {
			make_rule_add_act(make, mrun, make_create_cmd(make, MCMD(STR("@$(TARGET) $(ARGS)"))));
		}
	}

	if (partifact->flags & PROP_SET) {
		const make_rule_t rartifact = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("artifact"))), 0));
		make_rule_add_depend(make, rartifact, MRULE(MSTR(STR("check"))));
		make_rule_add_depend(make, rartifact, MRULE(MVAR(target)));
		make_rule_add_act(make, rartifact, make_create_cmd(make, MCMD(STR("@mkdir -p $(ARTIFACTDIR)"))));
		make_rule_add_act(make, rartifact, make_create_cmd(make, MCMD(strf("@cp $(TARGET) $(ARTIFACT)"))));
	}

	str_t cleans = strn(CSTR("@$(RM)"), 128);

	const make_rule_t clean = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("clean"))), 0));

	str_catc(&cleans, CSTR(" $(TARGET)"));

	if (martifact != MAKE_END) {
		str_catc(&cleans, CSTR(" $(ARTIFACT)"));
	}

	if (lang & (1 << LANG_ASM)) {
		str_catc(&cleans, CSTR(" $(OBJ_ASM)"));
	}

	if (lang & (1 << LANG_C)) {
		str_catc(&cleans, CSTR(" $(OBJ_C)"));
	}

	if (lang & (1 << LANG_CPP)) {
		str_catc(&cleans, CSTR(" $(OBJ_CPP)"));
	}

	if (lang & (((1 << LANG_C) | (1 << LANG_CPP)))) {
		str_catc(&cleans, CSTR(" $(COV)"));
	}

	make_rule_add_act(make, clean, make_create_cmd(make, MCMD(cleans)));

	return 0;
}

static int gen_url(const proj_t *proj, make_t *make)
{
	const prop_str_t *name	 = proj->name;
	const prop_str_t *url	 = &proj->props[PROJ_PROP_URL].value;
	const prop_str_t *format = &proj->props[PROJ_PROP_FORMAT].value;
	const prop_str_t *config = &proj->props[PROJ_PROP_CONFIG].value;
	const prop_str_t *target = &proj->props[PROJ_PROP_TARGET].value;
	const prop_t *requires	 = &proj->props[PROJ_PROP_REQUIRE];

	const make_var_t murl = make_add_act(make, make_create_var(make, STR("URL"), MAKE_VAR_INST));
	make_var_add_val(make, murl, MSTR(strc(url->data, url->len)));

	const make_var_t mname = make_add_act(make, make_create_var(make, STR("NAME"), MAKE_VAR_INST));
	make_var_add_val(make, mname, MSTR(strc(name->data, name->len)));

	const make_var_t mformat = make_add_act(make, make_create_var(make, STR("FORMAT"), MAKE_VAR_INST));
	make_var_add_val(make, mformat, MSTR(strc(format->data, format->len)));

	const make_var_t mfile = make_add_act(make, make_create_var(make, STR("FILE"), MAKE_VAR_INST));
	make_var_add_val(make, mfile, MSTR(STR("$(NAME).$(FORMAT)")));

	const make_var_t dldir = make_add_act(make, make_create_var(make, STR("DLDIR"), MAKE_VAR_INST));
	make_var_add_val(make, dldir, MSTR(STR("$(SLNDIR)/dl/$(FILE)")));

	const make_var_t srcdir = make_add_act(make, make_create_var(make, STR("SRCDIR"), MAKE_VAR_INST));
	make_var_add_val(make, srcdir, MSTR(STR("$(SLNDIR)/staging/$(NAME)")));

	const make_var_t biulddir = make_add_act(make, make_create_var(make, STR("BUILDDIR"), MAKE_VAR_INST));
	make_var_add_val(make, biulddir, MSTR(STR("$(SLNDIR)/build/$(PLATFORM)/$(NAME)")));

	const make_var_t logdir = make_add_act(make, make_create_var(make, STR("LOGDIR"), MAKE_VAR_INST));
	make_var_add_val(make, logdir, MSTR(STR("$(SLNDIR)/logs/$(PLATFORM)/$(NAME)")));

	make_add_act(make, make_create_empty(make));

	const make_rule_t all = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("all"))), 0));
	make_rule_add_depend(make, all, MRULE(MSTR(STR("compile"))));

	const make_rule_t check = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("check"))), 0));

	const make_if_t if_curl = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which curl)"))));
	make_if_add_true_act(make, if_curl, make_create_cmd(make, MCMD(STR("sudo apt install curl"))));

	if (requires->flags & PROP_SET) {
		for (uint i = 0; i < requires->arr.cnt; i++) {
			const prop_str_t *require = arr_get(&requires->arr, i);

			make_if_t if_dpkg =
				make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(strf("$(shell dpkg -l %.*s)", require->len, require->data))));
			make_if_add_true_act(make, if_dpkg, make_create_cmd(make, MCMD(strf("sudo apt install %.*s", require->len, require->data))));
		}
	}

	const make_rule_t rdldir = make_add_act(make, make_create_rule(make, MRULE(MVAR(dldir)), 1));
	make_rule_add_act(make, rdldir, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
	make_rule_add_act(make, rdldir, make_create_cmd(make, MCMD(STR("@cd $(@D) && curl -O $(URL)$(FILE)"))));

	const make_rule_t rsrcdir = make_add_act(make, make_create_rule(make, MRULEACT(MVAR(srcdir), STR("done")), 1));
	make_rule_add_depend(make, rsrcdir, MRULE(MVAR(dldir)));
	make_rule_add_act(make, rsrcdir, make_create_cmd(make, MCMD(STR("@mkdir -p $(SLNDIR)/staging"))));
	make_rule_add_act(make, rsrcdir, make_create_cmd(make, MCMD(STR("@tar xf $(DLDIR) -C $(SLNDIR)/staging"))));
	make_rule_add_act(make, rsrcdir, make_create_cmd(make, MCMD(STR("@touch $(SRCDIR)/done"))));

	const make_rule_t routdir = make_add_act(make, make_create_rule(make, MRULEACT(MSTR(STR("$(OUTDIR)")), STR("$(NAME)")), 1));
	make_rule_add_depend(make, routdir, MRULEACT(MVAR(srcdir), STR("done")));
	make_rule_add_act(make, routdir, make_create_cmd(make, MCMD(STR("@mkdir -p $(LOGDIR) $(BUILDDIR) $(OUTDIR)"))));
	make_rule_add_act(
		make, routdir,
		make_create_cmd(make,
				MCMD(strf("@cd $(BUILDDIR) && $(SRCDIR)/configure --target=$(PLATFORM)-elf --prefix=$(OUTDIR) %.*s 2>&1 | tee $(LOGDIR)/configure.log",
					  config->len, config->data))));
	make_rule_add_act(make, routdir, make_create_cmd(make, MCMD(strf("@cd $(BUILDDIR) && make %.*s 2>&1 | tee $(LOGDIR)/make.log", target->len, target->data))));
	make_rule_add_act(make, routdir, make_create_cmd(make, MCMD(STR("@touch $(OUTDIR)/$(NAME)"))));

	const make_rule_t compile = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("compile"))), 0));
	make_rule_add_depend(make, compile, MRULE(MSTR(STR("check"))));
	make_rule_add_depend(make, compile, MRULEACT(MSTR(STR("$(OUTDIR)")), STR("$(NAME)")));

	make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("clean"))), 0));

	return 0;
}

int mk_proj_gen(proj_t *proj, const dict_t *projects, const path_t *path, const prop_t *sln_props)
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
	if (path_child(&cmake_path, proj->rel_path.path, proj->rel_path.len) == NULL) {
		return 1;
	}

	if (!folder_exists(cmake_path.path)) {
		folder_create(cmake_path.path);
	}

	if (path_child(&cmake_path, CSTR("Makefile")) == NULL) {
		return 1;
	}

	MSG("generating project: %s", cmake_path.path);

	make_init(&proj->make, 8, 8, 8);

	make_create_var_ext(&proj->make, STR("SLNDIR"), MAKE_VAR_INST);
	const make_var_t mplatform = make_create_var_ext(&proj->make, STR("PLATFORM"), MAKE_VAR_INST);
	const make_var_t mconfig   = make_create_var_ext(&proj->make, STR("CONFIG"), MAKE_VAR_INST);

	make_var_t moutdir = MAKE_END;

	if (outdir->flags & PROP_SET) {
		buf_len = resolve(&outdir->value, CSTR(buf), proj);
		moutdir = make_add_act(&proj->make, make_create_var(&proj->make, STR("OUTDIR"), MAKE_VAR_INST));
		make_var_add_val(&proj->make, moutdir, MSTR(strn(buf, buf_len, buf_len + 1)));
	}

	if (url->flags & PROP_SET) {
		gen_url(proj, &proj->make);
	} else {
		gen_source(proj, projects, sln_props, &proj->make, mplatform, mconfig);
	}

	FILE *file = file_open(cmake_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	ret |= make_print(&proj->make, file);

	file_close(file);

	if (ret == 0) {
		SUC("generating project: %s success", cmake_path.path);
	} else {
		ERR("generating project: %s failed", cmake_path.path);
	}

	return ret;
}

void mk_proj_free(proj_t *proj)
{
	make_free(&proj->make);
}