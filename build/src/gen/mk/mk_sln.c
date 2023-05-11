#include "mk_sln.h"

#include "mk_proj.h"

#include "common.h"

typedef struct gen_proj_make_data_s {
	const path_t *path;
	const hashmap_t *projects;
	const prop_t *sln_props;
} gen_proj_make_data_t;

static void gen_proj_make(void *key, size_t ksize, void *value, const void *priv)
{
	const gen_proj_make_data_t *data = priv;
	mk_proj_gen(value, data->projects, data->path, data->sln_props);
}

static void add_phony_make(void *key, size_t ksize, void *value, void *priv)
{
	proj_t *proj = value;
	p_fprintf(priv, " %.*s", proj->name->len, proj->name->data);
}

static void print_action(FILE *file, const proj_t *proj, const prop_t *configs, const char *action)
{
	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len;

	buf_len = convert_slash(CSTR(buf), proj->rel_path.path, proj->rel_path.len);

	if (action == NULL) {
		p_fprintf(file, ".PHONY: %.*s\n%.*s: check", proj->name->len, proj->name->data, proj->name->len, proj->name->data);
	} else {
		p_fprintf(file, ".PHONY: %.*s/%s\n%.*s/%s: check", proj->name->len, proj->name->data, action, proj->name->len, proj->name->data, action);
	}

	if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE) {
		for (int i = 0; i < proj->all_depends.count; i++) {
			const proj_t *dproj = *(proj_t **)array_get(&proj->all_depends, i);

			if (dproj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
				if (action == NULL) {
					p_fprintf(file, " %.*s", dproj->name->len, dproj->name->data);
				} else {
					p_fprintf(file, " %.*s/%s", dproj->name->len, dproj->name->data, action);
				}
			}
		}
	}

	if (action == NULL) {
		p_fprintf(file, "\n\t@$(MAKE) -C %.*s SLNDIR=$(SLNDIR)", buf_len, buf);
	} else {
		p_fprintf(file, "\n\t@$(MAKE) -C %.*s %s SLNDIR=$(SLNDIR)", buf_len, buf, action);
	}

	if ((configs->flags & PROP_SET) && configs->arr.count > 0) {
		p_fprintf(file, " CONFIG=$(CONFIG)");
	}

	p_fprintf(file, "\n\n");
}

typedef struct add_target_make_data_s {
	const hashmap_t *projects;
	FILE *file;
	const prop_t *configs;
} add_target_make_data_t;

static void add_target_make(void *key, size_t ksize, void *value, void *priv)
{
	const add_target_make_data_t *data = priv;

	proj_t *proj = value;

	print_action(data->file, proj, data->configs, NULL);
	print_action(data->file, proj, data->configs, "clean");
	print_action(data->file, proj, data->configs, "compile");
	print_action(data->file, proj, data->configs, "coverage");
}

typedef struct add_clean_make_data_s {
	FILE *fp;
	const prop_t *configs;
} add_clean_make_data_t;

static void add_clean_make(void *key, size_t ksize, void *value, void *priv)
{
	const add_clean_make_data_t *data = priv;

	proj_t *proj = value;

	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len;

	buf_len = convert_slash(CSTR(buf), proj->rel_path.path, proj->rel_path.len);
	p_fprintf(data->fp, "\t@$(MAKE) -C %.*s clean SLNDIR=$(SLNDIR)", buf_len, buf);

	if ((data->configs->flags & PROP_SET) && data->configs->arr.count > 0) {
		p_fprintf(data->fp, " CONFIG=$(CONFIG)");
	}

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

	p_fprintf(file, "SLNDIR=$(CURDIR)\n\n");

	if ((configs->flags & PROP_SET) && configs->arr.count > 0) {
		p_fprintf(file, "CONFIGS =");

		for (int i = 0; i < configs->arr.count; i++) {
			const prop_str_t *config = array_get(&configs->arr, i);
			p_fprintf(file, " %.*s", config->len, config->data);
		}

		prop_str_t *config = array_get(&configs->arr, 0);
		p_fprintf(file, "\nCONFIG = %.*s\n\n", config->len, config->data);
	}

	p_fprintf(file, "SHOW = true\n\n");

	p_fprintf(file, ".PHONY: all check clean\n\nall:");

	hashmap_iterate_hc(&sln->projects, add_phony_make, file);

	p_fprintf(file, "\n\ncheck:\n");

	if ((configs->flags & PROP_SET) && configs->arr.count > 0) {
		p_fprintf(file, "ifeq ($(filter $(CONFIG),$(CONFIGS)),)\n"
				"\t$(error Config '$(CONFIG)' not found. Configs: $(CONFIGS))\n"
				"endif\n");
	}

	p_fprintf(file, "\n");

	add_target_make_data_t add_target_make_data = {
		.projects = &sln->projects,
		.file	  = file,
		.configs  = configs,
	};

	hashmap_iterate_hc(&sln->projects, add_target_make, &add_target_make_data);

	add_clean_make_data_t add_clean_make_data = {
		.fp	 = file,
		.configs = configs,
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
