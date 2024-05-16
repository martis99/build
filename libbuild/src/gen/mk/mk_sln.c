#include "gen/mk/mk_sln.h"

#include "gen/var.h"
#include "mk_proj.h"

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
	size_t buf_len = prop->val.len;
	mem_cpy(buf, buf_size, prop->val.data, prop->val.len);

	buf_len = invert_slash(buf, buf_len);
	buf_len = cstr_replaces(buf, buf_size, buf_len, vars.old, vars.new, __VAR_MAX, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_NAME)"), proj->name->val.data, proj->name->val.len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_FOLDER)"), proj->rel_path.path, proj->rel_path.len, NULL);

	return buf_len;
}

static void add_child_cmd(make_t *make, make_rule_t rule, const proj_t *proj, str_t target)
{
	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len;

	buf_len = convert_slash(CSTR(buf), proj->rel_path.path, proj->rel_path.len);
	make_rule_add_act(make, rule, make_create_cmd(make, MCMDCHILD(strn(buf, buf_len, buf_len + 1), target)));
}

static void add_action(make_t *make, const dict_t *projects, const proj_t *proj, bool add_depends, bool dep_compile, str_t action, int coverable)
{
	const make_rule_t maction = make_add_act(make, make_create_rule(make, MRULEACT(MSTR(strs(proj->name->val)), action), 0));
	make_rule_add_depend(make, maction, MRULE(MSTR(STR("check"))));

	if (dep_compile) {
		make_rule_add_depend(make, maction, MRULEACT(MSTR(strs(proj->name->val)), STR("compile")));
	}

	if (add_depends) {
		const arr_t *depends = &proj->props[PROJ_PROP_DEPENDS].arr;
		for (uint i = 0; i < depends->cnt; i++) {
			const prop_str_t *dname = arr_get(depends, i);

			const proj_t *dproj = NULL;
			if (dict_get(projects, dname->val.data, dname->val.len, (void **)&dproj)) {
				ERR("project doesn't exists: '%.*s'", (int)dname->val.len, dname->val.data);
				continue;
			}

			if (coverable == 0 || proj_coverable(dproj)) {
				if (str_eq(action, STR("coverage")) || str_eq(action, STR("coverage_dynamic"))) {
					make_rule_add_depend(make, maction, MRULEACT(MSTR(strs(dproj->name->val)), STR("coverage_static")));
				} else if (dproj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB && (str_eq(action, STR("compile")) || str_eq(action, STR("dynamic")))) {
					make_rule_add_depend(make, maction, MRULEACT(MSTR(strs(dproj->name->val)), STR("static")));
				} else {
					if (!(str_eq(action, STR("clean")) && dproj->props[PROJ_PROP_URL].flags & PROP_SET)) {
						make_rule_add_depend(make, maction, MRULEACT(MSTR(strs(dproj->name->val)), action));
					}
				}
			}
		}

		const arr_t *ddepends = &proj->props[PROJ_PROP_DDEPENDS].arr;
		for (uint i = 0; i < ddepends->cnt; i++) {
			const prop_str_t *dname = arr_get(ddepends, i);

			const proj_t *dproj = NULL;
			if (dict_get(projects, dname->val.data, dname->val.len, (void **)&dproj)) {
				ERR("project doesn't exists: '%.*s'", (int)dname->val.len, dname->val.data);
				continue;
			}

			if (coverable == 0 || proj_coverable(dproj)) {
				if (str_eq(action, STR("coverage")) || str_eq(action, STR("coverage_static"))) {
					make_rule_add_depend(make, maction, MRULEACT(MSTR(strs(dproj->name->val)), STR("coverage_dynamic")));
				} else if (dproj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB && (str_eq(action, STR("compile")) || str_eq(action, STR("static")))) {
					make_rule_add_depend(make, maction, MRULEACT(MSTR(strs(dproj->name->val)), STR("dynamic")));
				} else {
					make_rule_add_depend(make, maction, MRULEACT(MSTR(strs(dproj->name->val)), action));
				}
			}
		}
	}

	add_child_cmd(make, maction, proj, action);
}

int mk_sln_gen(sln_t *sln, const path_t *path)
{
	if (!folder_exists(path->path)) {
		ERR("folder does not exists: %.*s", (int)path->len, path->path);
		return 1;
	}

	const prop_t *configs = &sln->props[SLN_PROP_CONFIGS];

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

	const make_var_t slndir = make_add_act(&make, make_create_var(&make, STR("SLNDIR"), MAKE_VAR_INST));
	make_var_add_val(&make, slndir, MVAR(curdir));
	const make_var_t tld = make_add_act(&make, make_create_var(&make, STR("TLD"), MAKE_VAR_INST));
	make_var_add_val(&make, tld, MVAR(ld));
	const make_var_t tcc = make_add_act(&make, make_create_var(&make, STR("TCC"), MAKE_VAR_INST));
	make_var_add_val(&make, tcc, MVAR(cc));

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

	const make_var_t show = make_add_act(&make, make_create_var(&make, STR("SHOW"), MAKE_VAR_INST));
	make_var_add_val(&make, show, MSTR(STR("true")));

	make_add_act(&make, make_create_empty(&make));

	const make_rule_t all = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STR("all"))), 0));
	{
		const proj_t **proj;
		arr_foreach(&sln->build_order, proj)
		{
			if ((*proj)->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
				make_rule_add_depend(&make, all, MRULEACT(MSTR(strs((*proj)->name->val)), STR("static")));
				make_rule_add_depend(&make, all, MRULEACT(MSTR(strs((*proj)->name->val)), STR("dynamic")));

			} else {
				make_rule_add_depend(&make, all, MRULEACT(MSTR(strs((*proj)->name->val)), STR("compile")));
			}
		}
	}

	const make_rule_t check = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STR("check"))), 0));
	if ((configs->flags & PROP_SET) && configs->arr.cnt > 0) {
		make_if_t mif = make_rule_add_act(&make, check, make_create_if(&make, MSTR(STR("$(filter $(CONFIG),$(CONFIGS))")), MSTR(str_null())));
		make_if_add_true_act(&make, mif, make_create_cmd(&make, MCMDERR(STR("Config '$(CONFIG)' not found. Configs: $(CONFIGS)"))));
	}

	int artifacts = 0;
	{
		const proj_t **pproj;
		arr_foreach(&sln->build_order, pproj)
		{
			const proj_t *proj = *pproj;

			add_action(&make, &sln->projects, proj, 1, 0, STR("clean"), 0);

			if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
				add_action(&make, &sln->projects, proj, 1, 0, STR("static"), 0);
				add_action(&make, &sln->projects, proj, 1, 0, STR("dynamic"), 0);
			} else {
				add_action(&make, &sln->projects, proj, 1, 0, STR("compile"), 0);
			}

			if (proj_runnable(proj)) {
				add_action(&make, &sln->projects, proj, 0, 1, STR("run"), 0);
			}

			if (proj_coverable(proj)) {
				if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
					add_action(&make, &sln->projects, proj, 1, 0, STR("coverage_static"), 1);
					add_action(&make, &sln->projects, proj, 1, 0, STR("coverage_dynamic"), 1);
				} else {
					add_action(&make, &sln->projects, proj, 1, 0, STR("coverage"), 1);
				}
			}

			if (proj->props[PROJ_PROP_ARTIFACT].flags & PROP_SET) {
				add_action(&make, &sln->projects, proj, 0, 1, STR("artifact"), 0);
				artifacts++;
			}
		}
	}

	if (artifacts > 0) {
		const make_rule_t rartifact = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STR("artifact"))), 0));
		const proj_t **pproj;
		arr_foreach(&sln->build_order, pproj)
		{
			const proj_t *proj = *pproj;

			if (proj->props[PROJ_PROP_ARTIFACT].flags & PROP_SET) {
				make_rule_add_depend(&make, rartifact, MRULEACT(MSTR(strs(proj->name->val)), STR("artifact")));
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

	{
		proj_t **pproj;
		arr_foreach(&sln->build_order, pproj)
		{
			ret |= mk_proj_gen(*pproj, &sln->projects, path, sln->props);
		}
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
