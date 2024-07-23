#include "proj_gen.h"

#include "common.h"

#include "gen/sln.h"

static int gen_source(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, resolve_fn resolve, resolve_path_fn resolve_path, pgc_t *pgc)
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

	switch (proj->props[PROJ_PROP_TYPE].mask) {
	case PROJ_TYPE_EXE:
		pgc->builds = F_PGC_BUILD_EXE;
		break;
	case PROJ_TYPE_BIN:
		pgc->builds = F_PGC_BUILD_BIN;
		break;
	case PROJ_TYPE_ELF:
		pgc->builds = F_PGC_BUILD_ELF;
		break;
	case PROJ_TYPE_FAT12:
		pgc->builds = F_PGC_BUILD_FAT12;
		break;
	case PROJ_TYPE_LIB:
		pgc->builds = F_PGC_BUILD_STATIC | F_PGC_BUILD_SHARED;
		break;
	case PROJ_TYPE_EXT:
		break;
	}

	const prop_t *outdir = &proj->props[PROJ_PROP_OUTDIR];
	if (outdir->flags & PROP_SET) {
		pgc->str[PGC_STR_OUTDIR] = str_cpy(resolve(outdir->value.val, &buf, proj));
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

	if (proj->props[PROJ_PROP_SOURCE].arr.cnt > 0) {
		const prop_t *intdir = &proj->props[PROJ_PROP_INTDIR];
		if (intdir->flags & PROP_SET) {
			resolve(intdir->value.val, &buf, proj);
			switch (proj->props[PROJ_PROP_TYPE].mask) {
			case PROJ_TYPE_EXE:
			case PROJ_TYPE_BIN:
			case PROJ_TYPE_ELF:
				pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = str_cpy(buf);
				break;
			case PROJ_TYPE_LIB: {
				resolve(intdir->value.val, &buf, proj);
				str_t s = strn(buf.data, buf.len, buf.len + 8);
				str_cat(&s, STR("static/"));
				pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_STATIC] = s;

				str_t d = strn(buf.data, buf.len, buf.len + 8);
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
		pgc->str[PGC_STR_ARGS] = str_cpy(resolve(proj->props[PROJ_PROP_ARGS].value.val, &buf, proj));
	}

	arr_foreach(&proj->props[PROJ_PROP_SOURCE].arr, source)
	{
		pgc_add_include(pgc, str_cpy(resolve(source->val, &buf, proj)));
	}

	arr_foreach(&proj->props[PROJ_PROP_INCLUDE].arr, include)
	{
		pgc_add_include(pgc, str_cpy(resolve(include->val, &buf, proj)));
	}

	for (uint i = 0; i < proj->includes.cnt; i++) {
		const proj_t *iproj = *(proj_t **)arr_get(&proj->includes, i);

		str_t rel = strc(iproj->rel_dir.path, iproj->rel_dir.len);

		arr_foreach(&iproj->props[PROJ_PROP_INCLUDE].arr, include)
		{
			pgc_add_include(pgc, str_cpy(resolve(resolve_path(rel, include->val, &buf), &buf, iproj)));
		}

		if (iproj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
			pgc_add_include(pgc, str_cpy(resolve(resolve_path(rel, iproj->props[PROJ_PROP_ENCLUDE].value.val, &buf), &buf, iproj)));
		}
	}

	if (proj->props[PROJ_PROP_DEFINES].flags & PROP_SET) {
		const arr_t *defines = &proj->props[PROJ_PROP_DEFINES].arr;

		for (uint k = 0; k < defines->cnt; k++) {
			const prop_str_t *define = arr_get(defines, k);
			pgc_add_define(pgc, resolve(define->val, &buf, proj), F_PGC_INTDIR_OBJECT | F_PGC_BUILD_STATIC | F_PGC_INTDIR_SHARED);
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
				const prop_t *doutdir = &dep->proj->props[PROJ_PROP_OUTDIR];

				if (doutdir->flags & PROP_SET) {
					resolve(resolve_path(rel, doutdir->value.val, &buf), &buf, dep->proj);

					if (dep->link_type == LINK_TYPE_SHARED) {
						pgc_add_lib(pgc, str_cpy(buf), str_cpy(dep->proj->name), PGC_LINK_SHARED, PGC_LIB_INT);
						str_cat(&buf, dep->proj->name);
						str_cat(&buf, STR(".so"));
						pgc_add_copyfile(pgc, str_cpy(buf));
					} else {
						pgc_add_lib(pgc, str_cpy(buf), str_cpy(dep->proj->name), PGC_LINK_STATIC, PGC_LIB_INT);
					}
				}
			} else {
				const prop_t *lib = &dep->proj->props[PROJ_PROP_LIB];
				if (dep->link_type == LINK_TYPE_STATIC && lib->flags & PROP_SET) {
					resolve(resolve_path(rel, STR(""), &buf), &buf, dep->proj);
					pgc_add_lib(pgc, str_cpy(buf), strs(lib->value.val), PGC_LINK_STATIC, PGC_LIB_EXT);
				}

				const prop_t *dlib = &dep->proj->props[PROJ_PROP_DLIB];
				if (dep->link_type == LINK_TYPE_SHARED && dlib->flags & PROP_SET) {
					resolve(resolve_path(rel, STR(""), &buf), &buf, dep->proj);
					pgc_add_lib(pgc, str_cpy(buf), str_cpy(dlib->value.val), PGC_LINK_SHARED, PGC_LIB_EXT);
					resolve(resolve_path(rel, dlib->value.val, &buf), &buf, dep->proj);
					pgc_add_copyfile(pgc, strf("%.*s.so", buf.len, buf.data));
				}
			}
		}

		const prop_str_t *libdir;
		arr_foreach(&dep->proj->props[PROJ_PROP_LIBDIRS].arr, libdir)
		{
			resolve(resolve_path(rel, libdir->val, &buf), &buf, dep->proj);
			pgc_add_lib(pgc, str_cpy(buf), str_null(), PGC_LINK_SHARED, PGC_LIB_EXT);
		}

		//TODO: Not needed anymore? Replaced with LIB
		const prop_str_t *link;
		arr_foreach(&dep->proj->props[PROJ_PROP_LINK].arr, link)
		{
			pgc_add_lib(pgc, str_null(), str_cpy(link->val), PGC_LINK_SHARED, PGC_LIB_EXT);
		}
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
		pgc_add_copyfile(pgc, str_cpy(resolve(copyfile->val, &buf, proj)));
	}

	const prop_t *header = &proj->props[PROJ_PROP_HEADER];
	if (header->flags & PROP_SET) {
		proj_t *fproj = NULL;
		if (dict_get(projects, header->value.val.data, header->value.val.len, (void **)&fproj)) {
			ERR("project doesn't exist: '%.*s'", (int)header->value.val.len, header->value.val.data);
		} else {
			str_cat(&tmp, fproj->props[PROJ_PROP_OUTDIR].value.val);
			if (fproj->props[PROJ_PROP_FILENAME].flags & PROP_SET) {
				str_cat(&tmp, fproj->props[PROJ_PROP_FILENAME].value.val);
			} else {
				str_cat(&tmp, fproj->name);
			}

			str_cat(&tmp, STR(".bin"));
			pgc->str[PGC_STR_HEADER] = str_cpy(resolve(tmp, &buf, fproj));
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

		tmp.len = 0;
		str_cat(&tmp, fproj->props[PROJ_PROP_OUTDIR].value.val);
		if (fproj->props[PROJ_PROP_FILENAME].flags & PROP_SET) {
			str_cat(&tmp, fproj->props[PROJ_PROP_FILENAME].value.val);
		} else {
			str_cat(&tmp, fproj->name);
		}

		switch (fproj->props[PROJ_PROP_TYPE].mask) {
		case PROJ_TYPE_BIN:
			str_cat(&tmp, STR(".bin"));
			pgc_add_file(pgc, str_cpy(resolve(tmp, &buf, fproj)), PGC_FILE_BIN);
			break;
		case PROJ_TYPE_ELF:
			str_cat(&tmp, STR(".elf"));
			pgc_add_file(pgc, str_cpy(resolve(tmp, &buf, fproj)), PGC_FILE_ELF);
			break;
		}
	}

	const prop_t *size = &proj->props[PROJ_PROP_SIZE];
	if (size->flags & PROP_SET) {
		pgc->str[PGC_STR_SIZE] = str_cpy(size->value.val);
	}

	const prop_t *run = &proj->props[PROJ_PROP_RUN];
	if (run->flags & PROP_SET) {
		pgc_set_run(pgc, str_cpy(resolve(run->value.val, &buf, proj)), pgc->builds);
	}

	const prop_t *drun = &proj->props[PROJ_PROP_DRUN];
	if (drun->flags & PROP_SET) {
		pgc_set_run_debug(pgc, str_cpy(resolve(drun->value.val, &buf, proj)), pgc->builds);
	}

	return 0;
}

static int gen_url(const proj_t *proj, resolve_fn resolve, resolve_path_fn resolve_path, pgc_t *pgc)
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
		pgc->str[PGC_STR_OUTDIR] = str_cpy(resolve(proj->props[PROJ_PROP_OUTDIR].value.val, &buf, proj));
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

int proj_gen(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, resolve_fn resolve, resolve_path_fn resolve_path, pgc_t *pgc)
{
	if (proj->props[PROJ_PROP_URL].flags & PROP_SET) {
		return gen_url(proj, resolve, resolve_path, pgc);
	}

	return gen_source(proj, projects, sln_props, resolve, resolve_path, pgc);
}
