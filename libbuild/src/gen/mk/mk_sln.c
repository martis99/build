#include "gen/mk/mk_sln.h"

#include "gen/var.h"
#include "mk_proj.h"

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

static size_t resolve(const prop_str_t *prop, char *buf, size_t buf_size, const proj_t *proj)
{
	size_t buf_len = prop->len;
	mem_cpy(CSTR(buf), prop->data, prop->len);

	buf_len = invert_slash(buf, buf_len);
	buf_len = cstr_replaces(buf, buf_size, buf_len, vars.old, vars.new, __VAR_MAX, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_NAME)"), proj->name->data, proj->name->len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_FOLDER)"), proj->rel_path.path, proj->rel_path.len, NULL);

	return buf_len;
}

static void print_action(FILE *file, const proj_t *proj, bool add_depends, bool dep_compile, const char *action)
{
	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len;

	buf_len = convert_slash(CSTR(buf), proj->rel_path.path, proj->rel_path.len);

	if (action == NULL) {
		c_fprintf(file, ".PHONY: %.*s\n%.*s: check", proj->name->len, proj->name->data, proj->name->len, proj->name->data);
	} else {
		c_fprintf(file, ".PHONY: %.*s/%s\n%.*s/%s: check", proj->name->len, proj->name->data, action, proj->name->len, proj->name->data, action);
	}

	if (dep_compile) {
		c_fprintf(file, " %.*s/compile", proj->name->len, proj->name->data);
	}

	const proj_type_t type = proj->props[PROJ_PROP_TYPE].mask;

	if (add_depends && (type == PROJ_TYPE_EXE || type == PROJ_TYPE_BIN || type == PROJ_TYPE_FAT12)) {
		for (uint i = 0; i < proj->all_depends.cnt; i++) {
			const proj_t *dproj = *(proj_t **)arr_get(&proj->all_depends, i);

			if (action == NULL) {
				c_fprintf(file, " %.*s", dproj->name->len, dproj->name->data);
			} else {
				c_fprintf(file, " %.*s/%s", dproj->name->len, dproj->name->data, action);
			}
		}
	}

	c_fprintf(file, "\n\t@$(MAKE) -C %.*s", buf_len, buf);

	if (action) {
		c_fprintf(file, " %s", action);
	}

	c_fprintf(file, "\n\n");
}

int mk_sln_gen(const sln_t *sln, const path_t *path)
{
	if (!folder_exists(path->path)) {
		ERR("folder does not exists: %.*s", (int)path->len, path->path);
		return 1;
	}

	const char *targets_folder = "CMake";
	const prop_t *configs	   = &sln->props[SLN_PROP_CONFIGS];

	path_t cmake_path = *path;
	if (path_child(&cmake_path, CSTR("Makefile")) == NULL) {
		return 1;
	}

	FILE *file = file_open(cmake_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	MSG("generating solution: %s", cmake_path.path);

	int ret = 0;

	c_fprintf(file, "SLNDIR := $(CURDIR)\n"
			"TLD := $(LD)\n"
			"TCC := $(CC)\n"
			"\n");

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

			buf_len = export->len;
			mem_cpy(CSTR(buf), export->data, export->len);
			buf_len = cstr_replace(buf, sizeof(buf), buf_len, CSTR("$(OUTDIR)"), out, out_len, NULL);

			c_fprintf(file, "%.*s\n", buf_len, buf);
		}
	}

	c_fprintf(file, "\nexport\n\n");

	if ((configs->flags & PROP_SET) && configs->arr.cnt > 0) {
		c_fprintf(file, "CONFIGS :=");

		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);
			c_fprintf(file, " %.*s", config->len, config->data);
		}

		prop_str_t *config = arr_get(&configs->arr, 0);
		c_fprintf(file, "\nCONFIG := %.*s\n\n", config->len, config->data);
	}

	c_fprintf(file, "SHOW := true\n\n");

	c_fprintf(file, ".PHONY: all check clean\n\nall:");

	dict_foreach(&sln->projects, pair)
	{
		const proj_t *proj = pair->value;
		c_fprintf(file, " %.*s", proj->name->len, proj->name->data);
	}

	c_fprintf(file, "\n\ncheck:\n");

	if ((configs->flags & PROP_SET) && configs->arr.cnt > 0) {
		c_fprintf(file, "ifeq ($(filter $(CONFIG),$(CONFIGS)),)\n"
				"\t$(error Config '$(CONFIG)' not found. Configs: $(CONFIGS))\n"
				"endif\n");
	}

	c_fprintf(file, "\n");

	dict_foreach(&sln->projects, pair)
	{
		const proj_t *proj = pair->value;

		const proj_type_t type = proj->props[PROJ_PROP_TYPE].mask;

		print_action(file, proj, 1, 0, NULL);
		print_action(file, proj, 1, 0, "clean");
		print_action(file, proj, 1, 0, "compile");
		if (type == PROJ_TYPE_EXE || type == PROJ_TYPE_BIN || type == PROJ_TYPE_FAT12) {
			print_action(file, proj, 0, 1, "run");
		}
		print_action(file, proj, 1, 0, "coverage");
	}

	c_fprintf(file, "clean: check\n");

	dict_foreach(&sln->projects, pair)
	{
		const proj_t *proj = pair->value;

		char buf[P_MAX_PATH] = { 0 };
		size_t buf_len;

		buf_len = convert_slash(CSTR(buf), proj->rel_path.path, proj->rel_path.len);
		c_fprintf(file, "\t@$(MAKE) -C %.*s clean", buf_len, buf);

		c_fprintf(file, "\n");
	}

	file_close(file);
	if (ret == 0) {
		SUC("generating solution: %s success", cmake_path.path);
	} else {
		ERR("generating solution: %s failed", cmake_path.path);
	}

	dict_foreach(&sln->projects, pair)
	{
		mk_proj_gen(pair->value, &sln->projects, path, sln->props);
	}

	return ret;
}
