#include "mk_sln.h"

#include "mk_proj.h"

#include "common.h"

typedef struct gen_proj_make_data_s {
	const path_t *path;
	const hashmap_t *projects;
	const prop_t *configs;
	const prop_t *langs;
	charset_t charset;
	const prop_t *cflags;
	const prop_t *outdir;
	const prop_t *intdir;
} gen_proj_make_data_t;

static void gen_proj_make(void *key, size_t ksize, void *value, const void *priv)
{
	const gen_proj_make_data_t *data = priv;
	mk_proj_gen(value, data->projects, data->path, data->configs, data->langs, data->cflags, data->outdir, data->intdir);
}

static void add_phony_make(void *key, size_t ksize, void *value, void *priv)
{
	proj_t *proj = value;
	p_fprintf(priv, " %.*s", proj->name->len, proj->name->data);
}

typedef struct add_target_make_data_s {
	const hashmap_t *projects;
	FILE *fp;
	const prop_t *configs;
} add_target_make_data_t;

static void add_target_make(void *key, size_t ksize, void *value, void *priv)
{
	const add_target_make_data_t *data = priv;

	proj_t *proj = value;

	char buf[P_MAX_PATH] = { 0 };

	convert_slash(buf, sizeof(buf) - 1, proj->rel_path.path, proj->rel_path.len);
	p_fprintf(data->fp, "%.*s: check", proj->name->len, proj->name->data);

	if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE) {
		for (int i = 0; i < proj->all_depends.count; i++) {
			const proj_t *dproj = *(proj_t **)array_get(&proj->all_depends, i);

			if (dproj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
				p_fprintf(data->fp, " %.*s", dproj->name->len, dproj->name->data);
			}
		}
	}

	p_fprintf(data->fp,
		  "\n"
		  "\t@$(MAKE) -C %.*s %.*s SLNDIR=$(SLNDIR)",
		  proj->rel_path.len, buf, proj->name->len, proj->name->data);

	if (data->configs->set && data->configs->arr.count > 0) {
		p_fprintf(data->fp, " CONFIG=$(CONFIG)");
	}

	p_fprintf(data->fp, "\n\n");
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

	convert_slash(buf, sizeof(buf) - 1, proj->rel_path.path, proj->rel_path.len);
	p_fprintf(data->fp, "\t@$(MAKE) -C %.*s clean SLNDIR=$(SLNDIR)", proj->rel_path.len, buf);

	if (data->configs->set && data->configs->arr.count > 0) {
		p_fprintf(data->fp, " CONFIG=$(CONFIG)");
	}

	p_fprintf(data->fp, "\n");
}

int mk_sln_gen(const sln_t *sln, const path_t *path)
{
	if (!folder_exists(path->path)) {
		ERR("folder does not exists: %.*s", (unsigned int)path->len, path->path);
		return 1;
	}

	const prop_str_t *name	   = &sln->props[SLN_PROP_NAME].value;
	const char *targets_folder = "CMake";
	const array_t *dirs	   = &sln->props[SLN_PROP_DIRS].arr;
	const prop_t *startup	   = &sln->props[SLN_PROP_STARTUP];
	const prop_t *configs	   = &sln->props[SLN_PROP_CONFIGS];

	path_t cmake_path = *path;
	if (path_child(&cmake_path, "Makefile", 8)) {
		return 1;
	}

	FILE *fp = file_open(cmake_path.path, "w", 1);
	if (fp == NULL) {
		return 1;
	}

	MSG("generating solution: %s", cmake_path.path);

	int ret = 0;

	gen_proj_make_data_t gen_proj_make_data = {
		.path	  = path,
		.projects = &sln->projects,
		.configs  = configs,
		.langs	  = &sln->props[SLN_PROP_LANGS],
		.cflags	  = &sln->props[SLN_PROP_CFLAGS],
		.outdir	  = &sln->props[SLN_PROP_OUTDIR],
		.intdir	  = &sln->props[SLN_PROP_INTDIR],
	};

	p_fprintf(fp, "SLNDIR=$(CURDIR)\n\n");

	if (configs->set && configs->arr.count > 0) {
		p_fprintf(fp, "CONFIGS =");

		for (int i = 0; i < configs->arr.count; i++) {
			const prop_str_t *config = array_get(&configs->arr, i);
			p_fprintf(fp, " %.*s", config->len, config->data);
		}

		prop_str_t *config = array_get(&configs->arr, 0);
		p_fprintf(fp, "\nCONFIG = %.*s\n\n", config->len, config->data);
	}

	p_fprintf(fp, ".PHONY: all check");

	hashmap_iterate_hc(&sln->projects, add_phony_make, fp);

	p_fprintf(fp, " clean\n"
		      "\n"
		      "all: clean");

	if (startup->set) {
		p_fprintf(fp, " %.*s", startup->value.len, startup->value.data);
	}

	p_fprintf(fp, "\n\ncheck:\n");

	if (configs->set && configs->arr.count > 0) {
		p_fprintf(fp, "ifeq ($(filter $(CONFIG),$(CONFIGS)),)\n"
			      "\t$(error Config '$(CONFIG)' not found. Configs: $(CONFIGS))\n"
			      "endif\n");
	}

	p_fprintf(fp, "\n");

	add_target_make_data_t add_target_make_data = {
		.projects = &sln->projects,
		.fp	  = fp,
		.configs  = configs,
	};

	hashmap_iterate_hc(&sln->projects, add_target_make, &add_target_make_data);

	add_clean_make_data_t add_clean_make_data = {
		.fp	 = fp,
		.configs = configs,
	};

	p_fprintf(fp, "clean: check\n");
	hashmap_iterate_hc(&sln->projects, add_clean_make, &add_clean_make_data);

	fclose(fp);
	if (ret == 0) {
		SUC("generating solution: %s success", cmake_path.path);
	} else {
		ERR("generating solution: %s failed", cmake_path.path);
	}

	hashmap_iterate_c(&sln->projects, gen_proj_make, &gen_proj_make_data);

	return ret;
}
