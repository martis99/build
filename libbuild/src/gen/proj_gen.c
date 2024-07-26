#include "proj_gen.h"

#include "common.h"

#include "gen/sln.h"
#include "pgc_common.h"

//TODO: Resolve when reading config file
static str_t resolve_path(str_t rel, str_t path, str_t *buf)
{
	if (str_eqn(path, STR("$(SLNDIR)"), 9)) {
		str_cpyd(path, buf);
		return *buf;
	}

	buf->len = 0;
	str_cat(buf, STR("$(SLNDIR)"));
	str_cat(buf, rel);
	str_cat(buf, path);
	return *buf;
}

static int add_target(pgc_t *pgc, str_t outdir, str_t name, str_t ext)
{
}

static int gen_source(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, pgc_t *pgc)
{
	char buf_d[P_MAX_PATH] = { 0 };
	char tmp_d[P_MAX_PATH] = { 0 };

	str_t buf = strb(buf_d, sizeof(buf_d), 0);
	str_t tmp = strb(tmp_d, sizeof(tmp_d), 0);

	int ret = 0;

	const str_t *filename = &proj->name;

	const prop_t *pfilename = &proj->props[PROJ_PROP_FILENAME];
	if (pfilename->flags & PROP_SET) {
		filename = &pfilename->value.val;
	}
	pgc->str[PGC_STR_NAME] = str_cpy(*filename);

	// clang-format off
	static struct {
		pgc_build_type_t build;
		int builds;
	} type_c[] = {
		[PROJ_TYPE_EXE]	  = { PGC_BUILD_EXE,    F_PGC_BUILD_EXE },
		[PROJ_TYPE_LIB]	  = { PGC_BUILD_STATIC, F_PGC_BUILD_STATIC | F_PGC_BUILD_SHARED},
		[PROJ_TYPE_ELF]	  = { PGC_BUILD_ELF,    F_PGC_BUILD_ELF }, 
		[PROJ_TYPE_BIN]	  = { PGC_BUILD_BIN,    F_PGC_BUILD_BIN }, 
		[PROJ_TYPE_FAT12] = { PGC_BUILD_FAT12,  F_PGC_BUILD_FAT12 },
	};

	static struct {
		const char *ext;
	} build_c[] = {
		[PGC_BUILD_EXE]	  = { ""     },
		[PGC_BUILD_STATIC]= { ".a"   },
		[PGC_BUILD_SHARED]= { ".so"  },
		[PGC_BUILD_ELF]	  = { ".elf" }, 
		[PGC_BUILD_BIN]	  = { ".bin" }, 
		[PGC_BUILD_FAT12] = { ".img" },
	};
	// clang-format on

	pgc->builds = type_c[proj->props[PROJ_PROP_TYPE].mask].builds;

	const prop_t *outdir = &proj->props[PROJ_PROP_OUTDIR];
	if (outdir->flags & PROP_SET) {
		pgc->str[PGC_STR_OUTDIR] = str_cpy(outdir->value.val);
		switch (proj->props[PROJ_PROP_TYPE].mask) {
		case PROJ_TYPE_EXE: {
			str_t c = strn(buf.data, buf.len, buf.len + 5);
			str_cat(&c, STR("cov/"));
			pgc->str[PGC_STR_COVDIR] = c;
			break;
		}
		default:
			break;
		}
	}

	if (outdir->flags & PROP_SET && filename->data != NULL && pgc->builds) {
		tmp.len = 0;
		str_cat(&tmp, outdir->value.val);
		str_cat(&tmp, *filename);

		switch (proj->props[PROJ_PROP_TYPE].mask) {
		case PROJ_TYPE_LIB:
			pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_STATIC] =
				strf("%.*s%.*s%s", outdir->value.val.len, outdir->value.val.data, filename->len, filename->data, build_c[PGC_BUILD_STATIC].ext);
			pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_SHARED] =
				strf("%.*s%.*s%s", outdir->value.val.len, outdir->value.val.data, filename->len, filename->data, build_c[PGC_BUILD_SHARED].ext);
			break;
		default:
			pgc->target[PGC_TARGET_STR_TARGET][type_c[proj->props[PROJ_PROP_TYPE].mask].build] =
				strf("%.*s%.*s%s", outdir->value.val.len, outdir->value.val.data, filename->len, filename->data,
				     build_c[type_c[proj->props[PROJ_PROP_TYPE].mask].build].ext);
			break;
		}
	}

	if (proj->props[PROJ_PROP_SOURCE].arr.cnt > 0) {
		const prop_t *intdir = &proj->props[PROJ_PROP_INTDIR];
		if (intdir->flags & PROP_SET) {
			switch (proj->props[PROJ_PROP_TYPE].mask) {
			case PROJ_TYPE_EXE:
			case PROJ_TYPE_BIN:
			case PROJ_TYPE_ELF:
				pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = str_cpy(intdir->value.val);
				break;
			case PROJ_TYPE_LIB: {
				str_t s = strn(intdir->value.val.data, intdir->value.val.len, intdir->value.val.len + 8);
				str_cat(&s, STR("static/"));
				pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_STATIC] = s;

				str_t d = strn(intdir->value.val.data, intdir->value.val.len, intdir->value.val.len + 8);
				str_cat(&d, STR("shared/"));
				pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_SHARED] = d;
				break;
			}
			default:
				break;
			}
		}
	}

	const prop_str_t *arch;
	arr_foreach(&sln_props[SLN_PROP_ARCHS].arr, arch)
	{
		pgc_add_arch(pgc, strs(arch->val));
	}

	const prop_str_t *config;
	arr_foreach(&sln_props[SLN_PROP_CONFIGS].arr, config)
	{
		pgc_add_config(pgc, strs(config->val));
	}

	uint lang = proj->props[PROJ_PROP_LANGS].mask;
	const prop_str_t *include;
	arr_foreach(&proj->props[PROJ_PROP_INCLUDE].arr, include)
	{
		int exts = 0;
		exts |= lang & (1 << LANG_NASM) ? F_PGC_HEADER_INC : 0;
		exts |= lang & (1 << LANG_ASM) ? F_PGC_HEADER_INC : 0;
		exts |= lang & (1 << LANG_C) || lang & (1 << LANG_CPP) ? F_PGC_HEADER_H : 0;
		exts |= lang & (1 << LANG_CPP) ? F_PGC_HEADER_HPP : 0;

		if (exts) {
			pgc_add_header(pgc, strs(include->val), exts);
		}
	}

	const prop_str_t *source;
	arr_foreach(&proj->props[PROJ_PROP_SOURCE].arr, source)
	{
		int headers = 0;
		headers |= lang & (1 << LANG_NASM) ? F_PGC_HEADER_INC : 0;
		headers |= lang & (1 << LANG_ASM) ? F_PGC_HEADER_INC : 0;
		headers |= lang & (1 << LANG_C) || lang & (1 << LANG_CPP) ? F_PGC_HEADER_H : 0;
		headers |= lang & (1 << LANG_CPP) ? F_PGC_HEADER_HPP : 0;

		if (headers) {
			pgc_add_header(pgc, strs(source->val), headers);
		}

		int srcs = 0;
		srcs |= lang & (1 << LANG_NASM) ? F_PGC_SRC_NASM : 0;
		srcs |= lang & (1 << LANG_ASM) ? F_PGC_SRC_S : 0;
		srcs |= lang & (1 << LANG_C) ? F_PGC_SRC_C : 0;
		srcs |= lang & (1 << LANG_CPP) ? F_PGC_SRC_CPP : 0;

		if (srcs) {
			pgc_add_src(pgc, strs(source->val), srcs);
		}
	}

	const prop_t *partifact = &proj->props[PROJ_PROP_ARTIFACT];
	if (partifact->flags & PROP_SET) {
		//TODO: Different names for different build types
		pgc->target[PGC_TARGET_STR_ARTIFACT][PGC_BUILD_EXE]    = strs(partifact->value.val);
		pgc->target[PGC_TARGET_STR_ARTIFACT][PGC_BUILD_BIN]    = strs(partifact->value.val);
		pgc->target[PGC_TARGET_STR_ARTIFACT][PGC_BUILD_ELF]    = strs(partifact->value.val);
		pgc->target[PGC_TARGET_STR_ARTIFACT][PGC_BUILD_FAT12]  = strs(partifact->value.val);
		pgc->target[PGC_TARGET_STR_ARTIFACT][PGC_BUILD_STATIC] = strs(partifact->value.val);
		pgc->target[PGC_TARGET_STR_ARTIFACT][PGC_BUILD_SHARED] = strs(partifact->value.val);
	}

	if (proj->props[PROJ_PROP_ARGS].flags & PROP_SET) {
		pgc->str[PGC_STR_ARGS] = str_cpy(proj->props[PROJ_PROP_ARGS].value.val);
	}

	arr_foreach(&proj->props[PROJ_PROP_SOURCE].arr, source)
	{
		pgc_add_include(pgc, str_cpy(source->val));
	}

	arr_foreach(&proj->props[PROJ_PROP_INCLUDE].arr, include)
	{
		pgc_add_include(pgc, str_cpy(include->val));
	}

	for (uint i = 0; i < proj->includes.cnt; i++) {
		const proj_t *iproj = *(proj_t **)arr_get(&proj->includes, i);

		str_t rel = strc(iproj->rel_dir.path, iproj->rel_dir.len);

		str_t *inc;
		arr_foreach(&iproj->pgcr.arr[PGC_ARR_INCLUDES], inc)
		{
			pgc_add_include(pgc, str_cpy(resolve_path(rel, *inc, &buf)));
		}
	}

	if (proj->props[PROJ_PROP_DEFINES].flags & PROP_SET) {
		const arr_t *defines = &proj->props[PROJ_PROP_DEFINES].arr;

		for (uint k = 0; k < defines->cnt; k++) {
			const prop_str_t *define = arr_get(defines, k);
			pgc_add_define(pgc, define->val, F_PGC_INTDIR_OBJECT | F_PGC_BUILD_STATIC | F_PGC_INTDIR_SHARED);
		}
	}

	const prop_t *flags = &proj->props[PROJ_PROP_FLAGS];
	if ((flags->flags & PROP_SET)) {
		if (flags->mask & (1 << FLAG_STD_C99)) {
			pgc_add_flag(pgc, STR("-std=c99"), PGC_SRC_C);
		}
		if (flags->mask & (1 << FLAG_FREESTANDING)) {
			pgc_add_flag(pgc, STR("-ffreestanding"), PGC_SRC_C);
		}
	}

	pgc_add_flag(pgc, STR("-Wall -Wextra -Werror -pedantic"), F_PGC_SRC_C | F_PGC_SRC_CPP);

	const proj_dep_t *dep;
	arr_foreach(&proj->all_depends, dep)
	{
		if (dep->link_type != LINK_TYPE_SHARED || !(dep->proj->props[PROJ_PROP_SOURCE].flags & PROP_SET)) {
			continue;
		}

		//TODO: cleanup
		str_t define = strz(dep->proj->name.len + 5);
		str_to_upper(dep->proj->name, &define);
		str_cat(&define, STR("_DLL"));
		pgc_add_define(pgc, define, F_PGC_INTDIR_OBJECT | F_PGC_INTDIR_SHARED);
		str_free(&define);
	}

	//TODO: cleanup
	str_t define = strz(proj->name.len + 11);
	str_to_upper(proj->name, &define);
	str_cat(&define, STR("_BUILD_DLL"));
	pgc_add_define(pgc, define, F_PGC_INTDIR_SHARED);
	str_free(&define);

	arr_foreach(&proj->depends, dep)
	{
		if (!(dep->proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) || dep->link_type == LINK_TYPE_SHARED) {
			if (dep->link_type == LINK_TYPE_STATIC) {
				pgc_add_depend(pgc, strf("%.*s_s", dep->proj->name.len, dep->proj->name.data));
			} else {
				pgc_add_depend(pgc, strf("%.*s_d", dep->proj->name.len, dep->proj->name.data));
			}
		}
	}

	if (!(proj->props[PROJ_PROP_SOURCE].flags & PROP_SET)) {
		arr_foreach(&proj->depends, dep)
		{
			if (dep->proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
				if (dep->link_type == LINK_TYPE_STATIC) {
					pgc_add_depend(pgc, strf("%.*s_s", dep->proj->name.len, dep->proj->name.data));
				} else {
					pgc_add_depend(pgc, strf("%.*s_d", dep->proj->name.len, dep->proj->name.data));
				}
			} else {
				pgc_add_depend(pgc, str_cpy(dep->proj->name));
			}
		}
	}

	if (flags->flags & PROP_SET) {
		if (flags->mask & (1 << FLAG_WHOLEARCHIVE)) {
			pgc_add_ldflag(pgc, STR("-Wl,--whole-archive"));
		}
		if (flags->mask & (1 << FLAG_ALLOWMULTIPLEDEFINITION)) {
			pgc_add_ldflag(pgc, STR("-Wl,--allow-multiple-definition"));
		}
	}

	for (int i = proj->all_depends.cnt - 1; i >= 0; i--) {
		proj_dep_t *dep = arr_get(&proj->all_depends, i);

		str_t rel = strc(dep->proj->rel_dir.path, dep->proj->rel_dir.len);

		if (dep->proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
			if (dep->proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
				if (dep->proj->pgcr.str[PGC_STR_OUTDIR].data) {
					resolve_path(rel, dep->proj->pgcr.str[PGC_STR_OUTDIR], &buf);

					if (dep->link_type == LINK_TYPE_SHARED) {
						pgc_add_lib(pgc, str_cpy(buf), str_cpy(dep->proj->name), PGC_LINK_SHARED, PGC_LIB_INT);
						str_cat(&buf, dep->proj->name);
						str_cat(&buf, STR(".so"));
						pgc_add_copyfile(pgc, str_cpy(buf));
					} else {
						pgc_add_lib(pgc, str_cpy(buf), str_cpy(dep->proj->name), PGC_LINK_STATIC, PGC_LIB_INT);
					}
				}
			}
		}

		pgc_lib_data_t *lib;
		arr_foreach(&dep->proj->pgcr.arr[PGC_ARR_LIBS], lib)
		{
			pgc_add_lib(pgc, str_cpy(lib->dir), str_cpy(lib->name), dep->link_type == LINK_TYPE_STATIC ? PGC_LINK_STATIC : PGC_LINK_SHARED, lib->lib_type);
		}
	}

	str_t rel = strc(proj->rel_dir.path, proj->rel_dir.len);

	const prop_t *lib = &proj->props[PROJ_PROP_LIB];
	if (lib->flags & PROP_SET) {
		resolve_path(rel, STR(""), &buf);
		pgc_add_lib(pgc, str_cpy(buf), strs(lib->value.val), __PGC_LINK_TYPE_MAX, PGC_LIB_EXT);
	}

	const prop_t *dlib = &proj->props[PROJ_PROP_DLIB];
	if (dlib->flags & PROP_SET) {
		resolve_path(rel, STR(""), &buf);
		pgc_add_lib(pgc, str_cpy(buf), str_cpy(dlib->value.val), __PGC_LINK_TYPE_MAX, PGC_LIB_EXT);
		resolve_path(rel, dlib->value.val, &buf);
		pgc_add_copyfile(pgc, strf("%.*s.so", buf.len, buf.data));
	}

	const prop_str_t *libdir;
	arr_foreach(&proj->props[PROJ_PROP_LIBDIRS].arr, libdir)
	{
		resolve_path(rel, libdir->val, &buf);
		pgc_add_lib(pgc, str_cpy(libdir->val), str_null(), __PGC_LINK_TYPE_MAX, PGC_LIB_EXT);
	}

	if ((flags->flags & PROP_SET) && (flags->mask & (1 << FLAG_WHOLEARCHIVE))) {
		pgc_add_ldflag(pgc, STR("-Wl,--no-whole-archive"));
	}

	if (lang & (1 << LANG_CPP)) {
		pgc_add_ldflag(pgc, STR("-lstdc++"));
	}

	//TODO: Think of better solution
	if (flags->flags & PROP_SET) {
		if (flags->mask & (1 << FLAG_STATIC)) {
			pgc_add_ldflag(pgc, STR("-static"));
		}
		if (flags->mask & (1 << FLAG_NOSTARTFILES)) {
			pgc_add_ldflag(pgc, STR("-nostartfiles"));
		}
		if (flags->mask & (1 << FLAG_MATH)) {
			pgc_add_ldflag(pgc, STR("-lm"));
		}

		if (flags->mask & (1 << FLAG_X11)) {
			pgc_add_ldflag(pgc, STR("-lX11"));
		}

		if (flags->mask & (1 << FLAG_GL)) {
			pgc_add_ldflag(pgc, STR("-lGL"));
		}

		if (flags->mask & (1 << FLAG_GLX)) {
			pgc_add_ldflag(pgc, STR("-lGLX"));
		}
	}

	const prop_str_t *require;
	arr_foreach(&proj->props[PROJ_PROP_REQUIRE].arr, require)
	{
		pgc_add_require(pgc, str_cpy(require->val));
	}

	const prop_str_t *copyfile;
	arr_foreach(&proj->props[PROJ_PROP_COPYFILES].arr, copyfile)
	{
		pgc_add_copyfile(pgc, str_cpy(copyfile->val));
	}

	const prop_t *header = &proj->props[PROJ_PROP_HEADER];
	if (header->flags & PROP_SET) {
		proj_t *fproj = NULL;
		if (dict_get(projects, header->value.val.data, header->value.val.len, (void **)&fproj)) {
			ERR("project doesn't exist: '%.*s'", (int)header->value.val.len, header->value.val.data);
		} else {
			pgc->str[PGC_STR_HEADER] = str_cpy(fproj->pgcr.target[PGC_TARGET_STR_TARGET][type_c[fproj->props[PROJ_PROP_TYPE].mask].build]);
		}
	}

	const prop_str_t *fname;
	arr_foreach(&proj->props[PROJ_PROP_FILES].arr, fname)
	{
		proj_t *fproj = NULL;
		if (dict_get(projects, fname->val.data, fname->val.len, (void **)&fproj)) {
			ERR("project doesn't exist: '%.*s'", (int)fname->val.len, fname->val.data);
			continue;
		}

		pgc_add_file(pgc, str_cpy(fproj->pgcr.target[PGC_TARGET_STR_TARGET][type_c[fproj->props[PROJ_PROP_TYPE].mask].build]), PGC_FILE_ELF);
	}

	const prop_t *wdir = &proj->props[PROJ_PROP_WDIR];
	if (wdir->flags & PROP_SET) {
		pgc_set_cwd(pgc, str_cpy(wdir->value.val));
	} else {
		str_t cwd = strz(10 + proj->rel_dir.len);
		str_cat(&cwd, STR("$(SLNDIR)"));
		str_catc(&cwd, proj->rel_dir.path, proj->rel_dir.len);
		pgc_set_cwd(pgc, cwd);
	}

	const prop_t *size = &proj->props[PROJ_PROP_SIZE];
	if (size->flags & PROP_SET) {
		pgc->str[PGC_STR_SIZE] = str_cpy(size->value.val);
	}

	const prop_t *run = &proj->props[PROJ_PROP_RUN];
	if (run->flags & PROP_SET) {
		pgc_set_run(pgc, str_cpy(run->value.val), pgc->builds);
	}

	const prop_t *drun = &proj->props[PROJ_PROP_DRUN];
	if (drun->flags & PROP_SET) {
		pgc_set_run_debug(pgc, str_cpy(drun->value.val), pgc->builds);
	}

	return 0;
}

static int gen_url(const proj_t *proj, pgc_t *pgc)
{
	char buf_d[P_MAX_PATH] = { 0 };

	str_t buf = strb(buf_d, sizeof(buf_d), 0);

	pgc->str[PGC_STR_NAME] = str_cpy(proj->name);

	if (proj->props[PROJ_PROP_URL].flags & PROP_SET) {
		pgc->str[PGC_STR_URL] = str_cpy(proj->props[PROJ_PROP_URL].value.val);
	}

	if (proj->props[PROJ_PROP_FORMAT].flags & PROP_SET) {
		pgc->str[PGC_STR_FORMAT] = str_cpy(proj->props[PROJ_PROP_FORMAT].value.val);
	}

	if (proj->props[PROJ_PROP_OUTDIR].flags & PROP_SET) {
		pgc->str[PGC_STR_OUTDIR] = str_cpy(proj->props[PROJ_PROP_OUTDIR].value.val);
	}

	const prop_str_t *require;
	arr_foreach(&proj->props[PROJ_PROP_REQUIRE].arr, require)
	{
		pgc_add_require(pgc, str_cpy(require->val));
	}

	if (proj->props[PROJ_PROP_CONFIG].flags & PROP_SET) {
		pgc->str[PGC_STR_CONFIG] = str_cpy(proj->props[PROJ_PROP_CONFIG].value.val);
	}

	if (proj->props[PROJ_PROP_TARGET].flags & PROP_SET) {
		pgc->str[PGC_STR_TARGETS] = str_cpy(proj->props[PROJ_PROP_TARGET].value.val);
	}

	return 0;
}

int proj_gen(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, pgc_t *pgc)
{
	if (proj->props[PROJ_PROP_URL].flags & PROP_SET) {
		return gen_url(proj, pgc);
	}

	return gen_source(proj, projects, sln_props, pgc);
}
