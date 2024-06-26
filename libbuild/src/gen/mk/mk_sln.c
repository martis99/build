#include "gen/mk/mk_sln.h"

#include "gen/var.h"
#include "mk_proj.h"

#include "common.h"

#include "make.h"

static size_t resolve(const prop_str_t *prop, char *buf, size_t buf_size, const proj_t *proj)
{
	size_t buf_len = prop->val.len;
	mem_cpy(buf, buf_size, prop->val.data, prop->val.len);

	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(SLN_DIR)"), CSTR("$(SLNDIR)"), NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_DIR)"), proj->rel_dir.path, proj->rel_dir.len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_NAME)"), proj->name.data, proj->name.len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(CONFIG)"), CSTR("$(CONFIG)"), NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PLATFORM)"), CSTR("$(PLATFORM)"), NULL);
	convert_slash(buf, buf_len);

	return buf_len;
}

static void add_child_cmd(make_t *make, make_rule_t rule, const proj_t *proj, str_t target)
{
	char buf[P_MAX_PATH] = { 0 };

	str_t path = str_cpy(strc(proj->rel_dir.path, proj->rel_dir.len));
	convert_slash((char *)path.data, path.len);
	path.len--;
	make_rule_add_act(make, rule, make_create_cmd(make, MCMDCHILD(path, target)));
}

static str_t get_action(const proj_t *proj, int shared)
{
	switch (proj->props[PROJ_PROP_TYPE].mask) {
	case PROJ_TYPE_LIB:
		return shared ? STR("/shared") : STR("/static");
	case PROJ_TYPE_BIN:
		return STR("/bin");
	case PROJ_TYPE_ELF:
		return STR("/elf");
	case PROJ_TYPE_FAT12:
		return STR("/fat12");
	default:
		return STR("/compile");
	}
}

static void add_compile_action(make_t *make, const dict_t *projects, const proj_t *proj, int shared)
{
	str_t action = get_action(proj, shared);

	const make_act_t act = make_add_act(make, make_create_rule(make, MRULEACT(MSTR(strs(proj->name)), action), 0));

	if (proj->depends.cnt == 0) {
		make_rule_add_depend(make, act, MRULE(MSTR(STR("check"))));
	}

	const proj_dep_t *dep;
	arr_foreach(&proj->depends, dep)
	{
		make_rule_add_depend(make, act, MRULEACT(MSTR(strs(dep->proj->name)), get_action(dep->proj, dep->link_type == LINK_TYPE_SHARED)));
	}

	add_child_cmd(make, act, proj, strc(action.data + 1, action.len - 1));
}

static void add_run_action(make_t *make, const dict_t *projects, const proj_t *proj, str_t action)
{
	const make_rule_t act = make_add_act(make, make_create_rule(make, MRULEACT(MSTR(strs(proj->name)), action), 0));

	make_rule_add_depend(make, act, MRULEACT(MSTR(strs(proj->name)), get_action(proj, 0)));

	add_child_cmd(make, act, proj, strc(action.data + 1, action.len - 1));
}

static void add_util_action(make_t *make, const dict_t *projects, const proj_t *proj, str_t action, int coverable)
{
	const make_rule_t maction = make_add_act(make, make_create_rule(make, MRULEACT(MSTR(strs(proj->name)), action), 0));
	make_rule_add_depend(make, maction, MRULE(MSTR(STR("check"))));

	const proj_dep_t *dep;
	arr_foreach(&proj->depends, dep)
	{
		make_rule_add_depend(make, maction, MRULEACT(MSTR(strs(dep->proj->name)), action));
	}

	add_child_cmd(make, maction, proj, strc(action.data + 1, action.len - 1));
}

int mk_sln_gen(sln_t *sln, const path_t *path)
{
	if (!folder_exists(path->path)) {
		ERR("folder does not exists: %.*s", (int)path->len, path->path);
		return 1;
	}

	const prop_t *configs = &sln->props[SLN_PROP_CONFIGS];
	const prop_t *archs   = &sln->props[SLN_PROP_ARCHS];

	path_t cmake_path = *path;
	if (path_child(&cmake_path, CSTR("Makefile")) == NULL) {
		return 1;
	}

	MSG("generating solution: %s", cmake_path.path);

	int ret = 0;

	make_t make = { 0 };
	make_init(&make, 16, 16, 16);

	const make_var_t curdir = make_add_act(&make, make_create_var_ext(&make, STR("CURDIR"), MAKE_VAR_INST));
	const make_var_t ld	= make_add_act(&make, make_create_var_ext(&make, STR("LD"), MAKE_VAR_INST));
	const make_var_t cc	= make_add_act(&make, make_create_var_ext(&make, STR("CC"), MAKE_VAR_INST));
	const make_var_t as	= make_add_act(&make, make_create_var_ext(&make, STR("AS"), MAKE_VAR_INST));

	const make_var_t slndir = make_add_act(&make, make_create_var(&make, STR("SLNDIR"), MAKE_VAR_INST));
	make_var_add_val(&make, slndir, MSTR(STR("$(CURDIR)/")));
	const make_var_t tld = make_add_act(&make, make_create_var(&make, STR("TLD"), MAKE_VAR_INST));
	make_var_add_val(&make, tld, MVAR(ld));
	const make_var_t tcc = make_add_act(&make, make_create_var(&make, STR("TCC"), MAKE_VAR_INST));
	make_var_add_val(&make, tcc, MVAR(cc));
	const make_var_t tas = make_add_act(&make, make_create_var(&make, STR("TAS"), MAKE_VAR_INST));
	make_var_add_val(&make, tas, MVAR(as));

	int first = 1;

	dict_foreach(&sln->projects, pair)
	{
		const proj_t *proj = pair->value;

		char buf[P_MAX_PATH] = { 0 };
		char out[P_MAX_PATH] = { 0 };

		size_t buf_len;
		size_t out_len;

		const prop_str_t *outdir = &proj->props[PROJ_PROP_OUTDIR].value;

		out_len = resolve(outdir, CSTR(out), proj);

		const prop_t *exports = &proj->props[PROJ_PROP_EXPORT];

		if (!(exports->flags & PROP_SET)) {
			continue;
		}

		for (uint i = 0; i < exports->arr.cnt; i++) {
			const prop_str_t *export = arr_get(&exports->arr, i);

			buf_len = export->val.len;
			mem_cpy(CSTR(buf), export->val.data, export->val.len);
			buf_len = cstr_replace(buf, sizeof(buf), buf_len, CSTR("$(OUTDIR)"), out, out_len, NULL);

			if (first) {
				make_add_act(&make, make_create_empty(&make));
				first = 0;
			}

			make_add_act(&make, make_create_cmd(&make, MCMD(strn(buf, buf_len, buf_len + 1))));
		}
	}

	make_add_act(&make, make_create_empty(&make));
	make_add_act(&make, make_create_cmd(&make, MCMD(STR("export"))));
	make_add_act(&make, make_create_empty(&make));

	if ((configs->flags & PROP_SET) && configs->arr.cnt > 0) {
		const make_var_t mconfigs = make_add_act(&make, make_create_var(&make, STR("CONFIGS"), MAKE_VAR_INST));

		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);
			make_var_add_val(&make, mconfigs, MSTR(strs(config->val)));
		}

		prop_str_t *config	 = arr_get(&configs->arr, 0);
		const make_var_t mconfig = make_add_act(&make, make_create_var(&make, STR("CONFIG"), MAKE_VAR_INST));
		make_var_add_val(&make, mconfig, MSTR(strs(config->val)));
	}

	make_add_act(&make, make_create_empty(&make));

	if ((archs->flags & PROP_SET) && archs->arr.cnt > 0) {
		const make_var_t marchs = make_add_act(&make, make_create_var(&make, STR("ARCHS"), MAKE_VAR_INST));

		for (uint i = 0; i < archs->arr.cnt; i++) {
			const prop_str_t *arch = arr_get(&archs->arr, i);
			make_var_add_val(&make, marchs, MSTR(strs(arch->val)));
		}

		prop_str_t *arch       = arr_get(&archs->arr, 0);
		const make_var_t march = make_add_act(&make, make_create_var(&make, STR("ARCH"), MAKE_VAR_INST));
		make_var_add_val(&make, march, MSTR(strs(arch->val)));
	}

	make_add_act(&make, make_create_empty(&make));

	const make_var_t show = make_add_act(&make, make_create_var(&make, STR("SHOW"), MAKE_VAR_INST));
	make_var_add_val(&make, show, MSTR(STR("true")));

	make_add_act(&make, make_create_empty(&make));

	const make_rule_t all = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STR("all"))), 0));
	{
		const proj_t **proj;
		arr_foreach(&sln->build_order, proj)
		{
			if ((*proj)->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
				make_rule_add_depend(&make, all, MRULEACT(MSTR(strs((*proj)->name)), STR("/static")));
				make_rule_add_depend(&make, all, MRULEACT(MSTR(strs((*proj)->name)), STR("/shared")));

			} else {
				make_rule_add_depend(&make, all, MRULEACT(MSTR(strs((*proj)->name)), STR("/compile")));
			}
		}
	}

	const make_rule_t check = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STR("check"))), 0));
	if ((configs->flags & PROP_SET) && configs->arr.cnt > 0) {
		make_if_t mif = make_rule_add_act(&make, check, make_create_if(&make, MSTR(STR("$(filter $(CONFIG),$(CONFIGS))")), MSTR(str_null())));
		make_if_add_true_act(&make, mif, make_create_cmd(&make, MCMDERR(STR("Config '$(CONFIG)' not found. Configs: $(CONFIGS)"))));
	}
	if ((archs->flags & PROP_SET) && archs->arr.cnt > 0) {
		make_if_t mif = make_rule_add_act(&make, check, make_create_if(&make, MSTR(STR("$(filter $(ARCH),$(ARCHS))")), MSTR(str_null())));
		make_if_add_true_act(&make, mif, make_create_cmd(&make, MCMDERR(STR("Arch '$(ARCH)' not found. Archs: $(ARCHS)"))));
	}

	int artifacts = 0;
	const proj_t **pproj;
	arr_foreach(&sln->build_order, pproj)
	{
		const proj_t *proj = *pproj;

		add_util_action(&make, &sln->projects, proj, STR("/clean"), 0);

		add_compile_action(&make, &sln->projects, proj, 0);
		if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
			add_compile_action(&make, &sln->projects, proj, 1);
		}

		if (proj_runnable(proj)) {
			add_run_action(&make, &sln->projects, proj, STR("/run"));
			add_run_action(&make, &sln->projects, proj, STR("/debug"));
		}

		if (proj_coverable(proj)) {
			add_util_action(&make, &sln->projects, proj, STR("/coverage"), 1);
		}

		if (proj->props[PROJ_PROP_ARTIFACT].flags & PROP_SET) {
			add_run_action(&make, &sln->projects, proj, STR("/artifact"));
			artifacts++;
		}
	}

	if (artifacts > 0) {
		const make_rule_t rartifact = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STR("artifact"))), 0));
		const proj_t **pproj;
		arr_foreach(&sln->build_order, pproj)
		{
			const proj_t *proj = *pproj;

			if (proj->props[PROJ_PROP_ARTIFACT].flags & PROP_SET) {
				make_rule_add_depend(&make, rartifact, MRULEACT(MSTR(strs(proj->name)), STR("/artifact")));
			}
		}
	}

	const make_rule_t clean = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STR("clean"))), 0));
	make_rule_add_depend(&make, clean, MRULE(MSTR(STR("check"))));

	dict_foreach(&sln->projects, pair)
	{
		add_child_cmd(&make, clean, pair->value, STR("clean"));
	}

	FILE *file = file_open(cmake_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	ret |= make_print(&make, PRINT_DST_FILE(file)) == 0;

	file_close(file);

	make_free(&make);

	if (ret == 0) {
		SUC("generating solution: %s success", cmake_path.path);
	} else {
		ERR("generating solution: %s failed", cmake_path.path);
	}

	arr_foreach(&sln->build_order, pproj)
	{
		ret |= mk_proj_gen(*(proj_t **)pproj, &sln->projects, sln->props);
	}

	return ret;
}

void mk_sln_free(sln_t *sln)
{
	dict_foreach(&sln->projects, pair)
	{
		mk_proj_free(pair->value);
	}
}
