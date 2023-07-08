#include "gen/mk/mk_sln.h"

#include "gen/var.h"
#include "mk_proj.h"

#include "common.h"

typedef struct gen_proj_make_data_s {
	const path_t *path;
	const hashmap_t *projects;
	const prop_t *sln_props;
} gen_proj_make_data_t;

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

static void gen_proj_make(void *key, size_t ksize, void *value, const void *priv)
{
	const gen_proj_make_data_t *data = priv;
	mk_proj_gen(value, data->projects, data->path, data->sln_props);
}

static void add_export(void *key, size_t ksize, void *value, void *priv)
{
	proj_t *proj = value;

	char buf[P_MAX_PATH] = { 0 };
	char out[P_MAX_PATH] = { 0 };

	size_t buf_len;
	size_t out_len;

	const prop_str_t *outdir = &proj->props[PROJ_PROP_OUTDIR].value;

	out_len = resolve(outdir, CSTR(out), proj);

	const prop_t *exports = &proj->props[PROJ_PROP_EXPORT];

	if (!(exports->flags & PROP_SET)) {
		return;
	}

	for (uint i = 0; i < exports->arr.cnt; i++) {
		const prop_str_t *export = arr_get(&exports->arr, i);

		buf_len = export->len;
		m_memcpy(CSTR(buf), export->data, export->len);

		buf_len = cstr_inplace(CSTR(buf), buf_len, CSTR("$(OUTDIR)"), out, out_len);

		p_fprintf(priv, "%.*s\n", buf_len, buf);
	}
}

static void add_phony_make(void *key, size_t ksize, void *value, void *priv)
{
	proj_t *proj = value;
	p_fprintf(priv, " %.*s", proj->name->len, proj->name->data);
}

static void print_action(FILE *file, const proj_t *proj, bool add_depends, bool dep_compile, const char *action)
{
	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len;

	buf_len = convert_slash(CSTR(buf), proj->rel_path.path, proj->rel_path.len);

	if (action == NULL) {
		p_fprintf(file, ".PHONY: %.*s\n%.*s: check", proj->name->len, proj->name->data, proj->name->len, proj->name->data);
	} else {
		p_fprintf(file, ".PHONY: %.*s/%s\n%.*s/%s: check", proj->name->len, proj->name->data, action, proj->name->len, proj->name->data, action);
	}

	if (dep_compile) {
		p_fprintf(file, " %.*s/compile", proj->name->len, proj->name->data);
	}

	if (add_depends && proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE) {
		for (uint i = 0; i < proj->all_depends.cnt; i++) {
			const proj_t *dproj = *(proj_t **)arr_get(&proj->all_depends, i);

			if (action == NULL) {
				p_fprintf(file, " %.*s", dproj->name->len, dproj->name->data);
			} else {
				p_fprintf(file, " %.*s/%s", dproj->name->len, dproj->name->data, action);
			}
		}
	}

	if (action == NULL) {
		p_fprintf(file, "\n\t@$(MAKE) -C %.*s", buf_len, buf);
	} else {
		p_fprintf(file, "\n\t@$(MAKE) -C %.*s %s", buf_len, buf, action);
	}

	p_fprintf(file, "\n\n");
}

typedef struct add_target_make_data_s {
	const hashmap_t *projects;
	FILE *file;
} add_target_make_data_t;

static void add_target_make(void *key, size_t ksize, void *value, void *priv)
{
	const add_target_make_data_t *data = priv;

	proj_t *proj = value;

	const proj_type_t type = proj->props[PROJ_PROP_TYPE].mask;

	print_action(data->file, proj, 1, 0, NULL);
	print_action(data->file, proj, 1, 0, "clean");
	print_action(data->file, proj, 1, 0, "compile");
	if (type == PROJ_TYPE_EXE) {
		print_action(data->file, proj, 0, 1, "run");
	}
	print_action(data->file, proj, 1, 0, "coverage");
}

typedef struct add_clean_make_data_s {
	FILE *fp;
} add_clean_make_data_t;

static void add_clean_make(void *key, size_t ksize, void *value, void *priv)
{
	const add_clean_make_data_t *data = priv;

	proj_t *proj = value;

	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len;

	buf_len = convert_slash(CSTR(buf), proj->rel_path.path, proj->rel_path.len);
	p_fprintf(data->fp, "\t@$(MAKE) -C %.*s clean", buf_len, buf);

	p_fprintf(data->fp, "\n");
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
	if (path_child(&cmake_path, CSTR("Makefile"))) {
		return 1;
	}

	FILE *file = file_open(cmake_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	MSG("generating solution: %s", cmake_path.path);

	int ret = 0;

	gen_proj_make_data_t gen_proj_make_data = {
		.path	   = path,
		.projects  = &sln->projects,
		.sln_props = sln->props,
	};

	p_fprintf(file, "SLNDIR = $(CURDIR)\n"
			"TLD = $(LD)\n"
			"TCC = $(CC)\n"
			"\n");

	hashmap_iterate_hc(&sln->projects, add_export, file);

	p_fprintf(file, "\nexport\n\n");

	if ((configs->flags & PROP_SET) && configs->arr.cnt > 0) {
		p_fprintf(file, "CONFIGS =");

		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);
			p_fprintf(file, " %.*s", config->len, config->data);
		}

		prop_str_t *config = arr_get(&configs->arr, 0);
		p_fprintf(file, "\nCONFIG = %.*s\n\n", config->len, config->data);
	}

	p_fprintf(file, "SHOW = true\n\n");

	p_fprintf(file, ".PHONY: all check clean\n\nall:");

	hashmap_iterate_hc(&sln->projects, add_phony_make, file);

	p_fprintf(file, "\n\ncheck:\n");

	if ((configs->flags & PROP_SET) && configs->arr.cnt > 0) {
		p_fprintf(file, "ifeq ($(filter $(CONFIG),$(CONFIGS)),)\n"
				"\t$(error Config '$(CONFIG)' not found. Configs: $(CONFIGS))\n"
				"endif\n");
	}

	p_fprintf(file, "\n");

	add_target_make_data_t add_target_make_data = {
		.projects = &sln->projects,
		.file	  = file,
	};

	hashmap_iterate_hc(&sln->projects, add_target_make, &add_target_make_data);

	add_clean_make_data_t add_clean_make_data = {
		.fp = file,
	};

	p_fprintf(file, "clean: check\n");
	hashmap_iterate_hc(&sln->projects, add_clean_make, &add_clean_make_data);

	file_close(file);
	if (ret == 0) {
		SUC("generating solution: %s success", cmake_path.path);
	} else {
		ERR("generating solution: %s failed", cmake_path.path);
	}

	hashmap_iterate_c(&sln->projects, gen_proj_make, &gen_proj_make_data);

	return ret;
}
