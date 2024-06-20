#include "mk_proj.h"

#include "gen/mk/mk_pgen.h"
#include "gen/sln.h"
#include "gen/var.h"

#include "common.h"

#include "make.h"

//TODO: Resolve when reading config file
static str_t resolve_path(str_t rel, str_t path, str_t *buf)
{
	if (str_eqn(path, STR("$(SLN_DIR)"), 9)) {
		str_cpyd(path, buf);
		return *buf;
	}

	buf->len = 0;
	str_cat(buf, STR("$(SLN_DIR)"));
	str_cat(buf, rel);
	str_cat(buf, path);
	return *buf;
}

static str_t resolve(str_t str, str_t *buf, const proj_t *proj)
{
	str_cpyd(str, buf);
	str_replace(buf, STR("$(SLN_DIR)"), STR("$(SLNDIR)"));
	str_replace(buf, STR("$(PROJ_DIR)"), strc(proj->rel_dir.path, proj->rel_dir.len));
	str_replace(buf, STR("$(PROJ_NAME)"), strc(proj->name.data, proj->name.len));
	str_replace(buf, STR("$(CONFIG)"), STR("$(CONFIG)"));
	str_replace(buf, STR("$(ARCH)"), STR("$(ARCH)"));
	convert_slash((char *)buf->data, buf->len);
	return *buf;
}

static int gen_source(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, mk_pgen_t *gen)
{
	char buf_d[P_MAX_PATH] = { 0 };

	str_t buf = strb(buf_d, sizeof(buf_d), 0);

	int ret = 0;

	const str_t *filename = &proj->name;

	const prop_t *pfilename = &proj->props[PROJ_PROP_FILENAME];
	if (pfilename->flags & PROP_SET) {
		filename = &pfilename->value.val;
	}
	gen->name = str_cpy(*filename);

	switch (proj->props[PROJ_PROP_TYPE].mask) {
	case PROJ_TYPE_EXE:
		gen->builds = F_MK_BUILD_EXE;
		break;
	case PROJ_TYPE_BIN:
		gen->builds = F_MK_BUILD_BIN;
		break;
	case PROJ_TYPE_ELF:
		gen->builds = F_MK_BUILD_ELF;
		break;
	case PROJ_TYPE_FAT12:
		gen->builds = F_MK_BUILD_FAT12;
		break;
	case PROJ_TYPE_LIB:
		gen->builds = F_MK_BUILD_STATIC | F_MK_BUILD_SHARED;
		break;
	case PROJ_TYPE_EXT:
		break;
	}

	const prop_t *outdir = &proj->props[PROJ_PROP_OUTDIR];
	if (outdir->flags & PROP_SET) {
		gen->outdir = str_cpy(resolve(outdir->value.val, &buf, proj));
		switch (proj->props[PROJ_PROP_TYPE].mask) {
		case PROJ_TYPE_EXE: {
			str_t c = strn(buf.data, buf.len, buf.len + 5);
			str_cat(&c, STR("cov/"));
			gen->covdir = c;
			break;
		}
		default:
			break;
		}
	}

	const prop_t *intdir = &proj->props[PROJ_PROP_INTDIR];
	if (intdir->flags & PROP_SET) {
		resolve(intdir->value.val, &buf, proj);
		switch (proj->props[PROJ_PROP_TYPE].mask) {
		case PROJ_TYPE_EXE:
		case PROJ_TYPE_BIN:
		case PROJ_TYPE_ELF:
			gen->intdir[MK_INTDIR_OBJECT] = str_cpy(buf);
			break;
		case PROJ_TYPE_LIB: {
			resolve(intdir->value.val, &buf, proj);
			str_t s = strn(buf.data, buf.len, buf.len + 8);
			str_cat(&s, STR("static/"));
			gen->intdir[MK_INTDIR_STATIC] = s;

			str_t d = strn(buf.data, buf.len, buf.len + 8);
			str_cat(&d, STR("shared/"));
			gen->intdir[MK_INTDIR_SHARED] = d;
			break;
		}
		default:
			break;
		}
	}

	const prop_str_t *config;
	arr_foreach(&sln_props[SLN_PROP_CONFIGS].arr, config)
	{
		mk_pgen_add_config(gen, strs(config->val));
	}

	uint lang = proj->props[PROJ_PROP_LANGS].mask;
	const prop_str_t *include;
	arr_foreach(&proj->props[PROJ_PROP_INCLUDE].arr, include)
	{
		int exts = 0;
		exts |= lang & (1 << LANG_NASM) ? F_MK_EXT_INC : 0;
		exts |= lang & (1 << LANG_ASM) ? F_MK_EXT_INC : 0;
		exts |= lang & (1 << LANG_C) || lang & (1 << LANG_CPP) ? F_MK_EXT_H : 0;
		exts |= lang & (1 << LANG_CPP) ? F_MK_EXT_HPP : 0;

		if (exts) {
			mk_pgen_add_header(gen, strs(include->val), exts);
		}
	}

	const prop_str_t *source;
	arr_foreach(&proj->props[PROJ_PROP_SOURCE].arr, source)
	{
		int headers = 0;
		headers |= lang & (1 << LANG_NASM) ? F_MK_EXT_INC : 0;
		headers |= lang & (1 << LANG_ASM) ? F_MK_EXT_INC : 0;
		headers |= lang & (1 << LANG_C) || lang & (1 << LANG_CPP) ? F_MK_EXT_H : 0;
		headers |= lang & (1 << LANG_CPP) ? F_MK_EXT_HPP : 0;

		if (headers) {
			mk_pgen_add_header(gen, strs(source->val), headers);
		}

		int srcs = 0;
		srcs |= lang & (1 << LANG_NASM) ? F_MK_EXT_ASM : 0;
		srcs |= lang & (1 << LANG_ASM) ? F_MK_EXT_S : 0;
		srcs |= lang & (1 << LANG_C) ? F_MK_EXT_C : 0;
		srcs |= lang & (1 << LANG_CPP) ? F_MK_EXT_CPP : 0;

		if (srcs) {
			mk_pgen_add_src(gen, strs(source->val), srcs);
		}
	}

	const prop_t *partifact = &proj->props[PROJ_PROP_ARTIFACT];
	if (partifact->flags & PROP_SET) {
		//TODO: Different names for different build types
		gen->artifact[MK_BUILD_EXE]    = strs(partifact->value.val);
		gen->artifact[MK_BUILD_BIN]    = strs(partifact->value.val);
		gen->artifact[MK_BUILD_ELF]    = strs(partifact->value.val);
		gen->artifact[MK_BUILD_FAT12]  = strs(partifact->value.val);
		gen->artifact[MK_BUILD_STATIC] = strs(partifact->value.val);
		gen->artifact[MK_BUILD_SHARED] = strs(partifact->value.val);
	}

	if (proj->props[PROJ_PROP_ARGS].flags & PROP_SET) {
		gen->args = str_cpy(resolve(proj->props[PROJ_PROP_ARGS].value.val, &buf, proj));
	}

	arr_foreach(&proj->props[PROJ_PROP_SOURCE].arr, source)
	{
		mk_pgen_add_include(gen, resolve(source->val, &buf, proj));
	}

	arr_foreach(&proj->props[PROJ_PROP_INCLUDE].arr, include)
	{
		mk_pgen_add_include(gen, resolve(include->val, &buf, proj));
	}

	for (uint i = 0; i < proj->includes.cnt; i++) {
		const proj_t *iproj = *(proj_t **)arr_get(&proj->includes, i);

		str_t rel = strc(iproj->rel_dir.path, iproj->rel_dir.len);

		if (iproj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
			const arr_t *includes = &iproj->props[PROJ_PROP_INCLUDE].arr;

			for (uint i = 0; i < includes->cnt; i++) {
				const prop_str_t *include = arr_get(includes, i);
				mk_pgen_add_include(gen, resolve_path(rel, include->val, &buf));
			}
		}

		if (iproj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
			mk_pgen_add_include(gen, resolve(resolve_path(rel, iproj->props[PROJ_PROP_ENCLUDE].value.val, &buf), &buf, iproj));
		}
	}

	if (proj->props[PROJ_PROP_DEFINES].flags & PROP_SET) {
		const arr_t *defines = &proj->props[PROJ_PROP_DEFINES].arr;

		for (uint k = 0; k < defines->cnt; k++) {
			const prop_str_t *define = arr_get(defines, k);
			mk_pgen_add_define(gen, define->val, F_MK_INTDIR_OBJECT | F_MK_BUILD_STATIC | F_MK_INTDIR_SHARED);
		}
	}

	const prop_t *flags = &proj->props[PROJ_PROP_FLAGS];
	if ((flags->flags & PROP_SET)) {
		if (flags->mask & (1 << FLAG_STD_C99)) {
			mk_pgen_add_flag(gen, STR("-std=c99"), MK_EXT_C);
		}
		if (flags->mask & (1 << FLAG_FREESTANDING)) {
			mk_pgen_add_flag(gen, STR("-ffreestanding"), MK_EXT_C);
		}
	}

	mk_pgen_add_flag(gen, STR("-Wall -Wextra -Werror -pedantic"), F_MK_EXT_C | F_MK_EXT_CPP);

	const proj_dep_t *dep;
	arr_foreach(&proj->all_depends, dep)
	{
		if (dep->link_type != LINK_TYPE_DYNAMIC) {
			continue;
		}

		//TODO: cleanup
		str_t define = strz(dep->proj->name.len + 5);
		str_to_upper(dep->proj->name, &define);
		str_cat(&define, STR("_DLL"));
		mk_pgen_add_define(gen, define, F_MK_INTDIR_OBJECT | F_MK_INTDIR_SHARED);
		str_free(&define);
	}

	//TODO: cleanup
	str_t define = strz(proj->name.len + 11);
	str_to_upper(proj->name, &define);
	str_cat(&define, STR("_BUILD_DLL"));
	mk_pgen_add_define(gen, define, F_MK_INTDIR_SHARED);
	str_free(&define);

	if (flags->flags & PROP_SET) {
		if (flags->mask & (1 << FLAG_WHOLEARCHIVE)) {
			mk_pgen_add_ldflag(gen, STR("-Wl,--whole-archive"));
		}
		if (flags->mask & (1 << FLAG_ALLOWMULTIPLEDEFINITION)) {
			mk_pgen_add_ldflag(gen, STR("-Wl,--allow-multiple-definition"));
		}
	}

	for (int i = proj->all_depends.cnt - 1; i >= 0; i--) {
		const proj_dep_t *dep = arr_get(&proj->all_depends, i);

		str_t rel = strc(dep->proj->rel_dir.path, dep->proj->rel_dir.len);

		if (dep->proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
			const prop_t *doutdir = &dep->proj->props[PROJ_PROP_OUTDIR];

			if (doutdir->flags & PROP_SET) {
				resolve(resolve_path(rel, doutdir->value.val, &buf), &buf, dep->proj);

				if (dep->link_type == LINK_TYPE_DYNAMIC) {
					mk_pgen_add_dlib(gen, strs(dep->proj->name));
					mk_pgen_add_dlib_dir(gen, buf);
				} else {
					mk_pgen_add_slib(gen, strs(dep->proj->name));
					mk_pgen_add_slib_dir(gen, buf);
				}
			}
		}

		const prop_str_t *libdir;
		arr_foreach(&dep->proj->props[PROJ_PROP_LIBDIRS].arr, libdir)
		{
			resolve(resolve_path(rel, libdir->val, &buf), &buf, dep->proj);
			mk_pgen_add_dlib_dir(gen, buf);
		}

		const prop_str_t *link;
		arr_foreach(&dep->proj->props[PROJ_PROP_LINK].arr, link)
		{
			mk_pgen_add_dlib(gen, link->val);
		}
	}

	if ((flags->flags & PROP_SET) && (flags->mask & (1 << FLAG_WHOLEARCHIVE))) {
		mk_pgen_add_ldflag(gen, STR("-Wl,--no-whole-archive"));
	}

	if (lang & (1 << LANG_CPP)) {
		mk_pgen_add_ldflag(gen, STR("-lstdc++"));
	}

	//TODO: Think of better solution
	if (flags->flags & PROP_SET) {
		if (flags->mask & (1 << FLAG_STATIC)) {
			mk_pgen_add_ldflag(gen, STR("-static"));
		}
		if (flags->mask & (1 << FLAG_NOSTARTFILES)) {
			mk_pgen_add_ldflag(gen, STR("-nostartfiles"));
		}
		if (flags->mask & (1 << FLAG_MATH)) {
			mk_pgen_add_ldflag(gen, STR("-lm"));
		}

		if (flags->mask & (1 << FLAG_X11)) {
			mk_pgen_add_ldflag(gen, STR("-lX11"));
		}

		if (flags->mask & (1 << FLAG_GL)) {
			mk_pgen_add_ldflag(gen, STR("-lGL"));
		}

		if (flags->mask & (1 << FLAG_GLX)) {
			mk_pgen_add_ldflag(gen, STR("-lGLX"));
		}
	}

	const prop_str_t *require;
	arr_foreach(&proj->props[PROJ_PROP_REQUIRE].arr, require)
	{
		mk_pgen_add_require(gen, require->val);
	}

	const prop_str_t *fname;
	arr_foreach(&proj->props[PROJ_PROP_FILES].arr, fname)
	{
		proj_t *fproj = NULL;
		if (dict_get(projects, fname->val.data, fname->val.len, (void **)&fproj)) {
			ERR("project doesn't exists: '%.*s'", (int)fname->val.len, fname->val.data);
			continue;
		}

		make_expand(&fproj->make);
		str_t mtarget = make_var_get_expanded(&fproj->make, STR("TARGET"));

		switch (fproj->props[PROJ_PROP_TYPE].mask) {
		case PROJ_TYPE_BIN:
			mk_pgen_add_file(gen, mtarget, MK_EXT_BIN);
			break;

		case PROJ_TYPE_ELF:
			mk_pgen_add_file(gen, mtarget, MK_EXT_ELF);
			break;
		}
	}

	const prop_t *size = &proj->props[PROJ_PROP_SIZE];
	if (size->flags & PROP_SET) {
		gen->size = str_cpy(size->value.val);
	}

	const prop_t *run = &proj->props[PROJ_PROP_RUN];
	if (run->flags & PROP_SET) {
		mk_pgen_set_run(gen, run->value.val, gen->builds);
	}

	const prop_t *drun = &proj->props[PROJ_PROP_DRUN];
	if (drun->flags & PROP_SET) {
		mk_pgen_set_run_debug(gen, drun->value.val, gen->builds);
	}

	return 0;
}

static int gen_url(const proj_t *proj, make_t *make)
{
	const str_t *name	 = &proj->name;
	const prop_str_t *url	 = &proj->props[PROJ_PROP_URL].value;
	const prop_str_t *format = &proj->props[PROJ_PROP_FORMAT].value;
	const prop_str_t *config = &proj->props[PROJ_PROP_CONFIG].value;
	const prop_str_t *target = &proj->props[PROJ_PROP_TARGET].value;
	const prop_t *requires	 = &proj->props[PROJ_PROP_REQUIRE];

	const make_var_t murl = make_add_act(make, make_create_var(make, STR("URL"), MAKE_VAR_INST));
	make_var_add_val(make, murl, MSTR(strs(url->val)));

	const make_var_t mname = make_add_act(make, make_create_var(make, STR("NAME"), MAKE_VAR_INST));
	make_var_add_val(make, mname, MSTR(strs(*name)));

	const make_var_t mformat = make_add_act(make, make_create_var(make, STR("FORMAT"), MAKE_VAR_INST));
	make_var_add_val(make, mformat, MSTR(strs(format->val)));

	const make_var_t mfile = make_add_act(make, make_create_var(make, STR("FILE"), MAKE_VAR_INST));
	make_var_add_val(make, mfile, MSTR(STR("$(NAME).$(FORMAT)")));

	const make_var_t dldir = make_add_act(make, make_create_var(make, STR("DLDIR"), MAKE_VAR_INST));
	make_var_add_val(make, dldir, MSTR(STR("$(SLNDIR)dl/$(FILE)/")));

	const make_var_t srcdir = make_add_act(make, make_create_var(make, STR("SRCDIR"), MAKE_VAR_INST));
	make_var_add_val(make, srcdir, MSTR(STR("$(SLNDIR)staging/$(NAME)/")));

	const make_var_t biulddir = make_add_act(make, make_create_var(make, STR("BUILDDIR"), MAKE_VAR_INST));
	make_var_add_val(make, biulddir, MSTR(STR("$(SLNDIR)build/$(ARCH)/$(NAME)/")));

	const make_var_t logdir = make_add_act(make, make_create_var(make, STR("LOGDIR"), MAKE_VAR_INST));
	make_var_add_val(make, logdir, MSTR(STR("$(SLNDIR)logs/$(ARCH)/$(NAME)/")));

	make_add_act(make, make_create_empty(make));

	const make_rule_t all = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("all"))), 0));
	make_rule_add_depend(make, all, MRULE(MSTR(STR("compile"))));

	const make_rule_t check = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("check"))), 0));

	const make_if_t if_curl = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which curl)"))));
	make_if_add_true_act(make, if_curl, make_create_cmd(make, MCMD(STR("sudo apt install curl -y"))));

	if (requires->flags & PROP_SET) {
		for (uint i = 0; i < requires->arr.cnt; i++) {
			const prop_str_t *require = arr_get(&requires->arr, i);

			make_if_t if_dpkg = make_rule_add_act(
				make, check, make_create_if(make, MSTR(str_null()), MSTR(strf("$(shell dpkg -l %.*s)", require->val.len, require->val.data))));
			make_if_add_true_act(make, if_dpkg, make_create_cmd(make, MCMD(strf("sudo apt install %.*s -y", require->val.len, require->val.data))));
		}
	}

	const make_rule_t rdldir = make_add_act(make, make_create_rule(make, MRULE(MVAR(dldir)), 1));
	make_rule_add_act(make, rdldir, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
	make_rule_add_act(make, rdldir, make_create_cmd(make, MCMD(STR("@cd $(@D) && curl -O $(URL)$(FILE)"))));

	const make_rule_t rsrcdir = make_add_act(make, make_create_rule(make, MRULEACT(MVAR(srcdir), STR("done")), 1));
	make_rule_add_depend(make, rsrcdir, MRULE(MVAR(dldir)));
	make_rule_add_act(make, rsrcdir, make_create_cmd(make, MCMD(STR("@mkdir -p $(SLNDIR)staging"))));
	make_rule_add_act(make, rsrcdir, make_create_cmd(make, MCMD(STR("@tar xf $(DLDIR) -C $(SLNDIR)staging"))));
	make_rule_add_act(make, rsrcdir, make_create_cmd(make, MCMD(STR("@touch $(SRCDIR)done"))));

	const make_rule_t routdir = make_add_act(make, make_create_rule(make, MRULEACT(MSTR(STR("$(OUTDIR)")), STR("$(NAME)")), 1));
	make_rule_add_depend(make, routdir, MRULEACT(MVAR(srcdir), STR("done")));
	make_rule_add_act(make, routdir, make_create_cmd(make, MCMD(STR("@mkdir -p $(LOGDIR) $(BUILDDIR) $(OUTDIR)"))));
	make_rule_add_act(
		make, routdir,
		make_create_cmd(make, MCMD(strf("@cd $(BUILDDIR) && $(SRCDIR)configure --target=$(ARCH)-elf --prefix=$(OUTDIR) %.*s 2>&1 | tee $(LOGDIR)configure.log",
						config->val.len, config->val.data))));
	make_rule_add_act(make, routdir,
			  make_create_cmd(make, MCMD(strf("@cd $(BUILDDIR) && make %.*s 2>&1 | tee $(LOGDIR)make.log", target->val.len, target->val.data))));
	make_rule_add_act(make, routdir, make_create_cmd(make, MCMD(STR("@touch $(OUTDIR)$(NAME)"))));

	const make_rule_t compile = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("compile"))), 0));
	make_rule_add_depend(make, compile, MRULE(MSTR(STR("check"))));
	make_rule_add_depend(make, compile, MRULEACT(MSTR(STR("$(OUTDIR)")), STR("$(NAME)")));

	make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("clean"))), 0));

	return 0;
}

int mk_proj_gen(proj_t *proj, const dict_t *projects, const prop_t *sln_props)
{
	const str_t *name = &proj->name;
	proj_type_t type  = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *url = &proj->props[PROJ_PROP_URL];

	char buf_d[P_MAX_PATH] = { 0 };

	str_t buf = strb(buf_d, sizeof(buf_d), 0);

	int ret = 0;

	path_t gen_path = { 0 };
	path_init(&gen_path, proj->dir.path, proj->dir.len);

	if (!folder_exists(gen_path.path)) {
		folder_create(gen_path.path);
	}

	if (path_child(&gen_path, CSTR("Makefile")) == NULL) {
		return 1;
	}

	MSG("generating project: %s", gen_path.path);

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_init(&proj->make, 8, 8, 8);

	if (url->flags & PROP_SET) {
		gen_url(proj, &proj->make);
	} else {
		gen_source(proj, projects, sln_props, &gen);
	}

	mk_pgen(&gen, &proj->make);

	mk_pgen_free(&gen);

	FILE *file = file_open(gen_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	ret |= make_print(&proj->make, PRINT_DST_FILE(file)) == 0;

	file_close(file);

	if (ret == 0) {
		SUC("generating project: %s success", gen_path.path);
	} else {
		ERR("generating project: %s failed", gen_path.path);
	}

	return ret;
}

void mk_proj_free(proj_t *proj)
{
	make_free(&proj->make);
}
//1064
