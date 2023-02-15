#include "mk_sln.h"

#include "mk_proj.h"

#include "defines.h"
#include "utils.h"

#include <stdio.h>

typedef struct gen_proj_make_data_s {
	const path_t *path;
	const hashmap_t *projects;
	int lang;
	charset_t charset;
} gen_proj_make_data_t;

static void gen_proj_make(void *key, size_t ksize, void *value, const void *usr)
{
	const gen_proj_make_data_t *data = usr;
	mk_proj_gen(value, data->projects, data->path);
}

static void add_phony_make(void *key, size_t ksize, void *value, void *usr)
{
	proj_t *proj = value;
	fprintf_s(usr, " %.*s", proj->props[PROJ_PROP_NAME].value.len, proj->props[PROJ_PROP_NAME].value.data);
}

static void add_target_make(void *key, size_t ksize, void *value, void *usr)
{
	proj_t *proj	   = value;
	char buf[MAX_PATH] = { 0 };
	convert_slash(buf, sizeof(buf) - 1, proj->rel_path.path, proj->rel_path.len);
	fprintf_s(usr,
		  "%.*s:\n"
		  "\t$(MAKE) -C %.*s SLNDIR=$(SLNDIR)\n"
		  "\n",
		  proj->props[PROJ_PROP_NAME].value.len, proj->props[PROJ_PROP_NAME].value.data, proj->rel_path.len, buf);
}

int mk_sln_gen(const sln_t *sln, const path_t *path)
{
	if (!folder_exists(path->path)) {
		ERR("folder does not exists: %.*s", (unsigned int)path->len, path->path);
		return 1;
	}

	const prop_str_t *name	   = &sln->props[SLN_PROP_NAME].value;
	unsigned int languages	   = sln->props[SLN_PROP_LANGS].mask;
	const char *targets_folder = "CMake";
	const array_t *dirs	   = &sln->props[SLN_PROP_DIRS].arr;
	const prop_str_t *startup  = &sln->props[SLN_PROP_STARTUP].value;

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
	};

	fprintf_s(fp, "SLNDIR=$(CURDIR)\n"
		      "\n"
		      ".PHONY: ");

	hashmap_iterate_hc(&sln->projects, add_phony_make, fp);

	fprintf_s(fp, "\n\n");

	hashmap_iterate_hc(&sln->projects, add_target_make, fp);

	fclose(fp);
	if (ret == 0) {
		SUC("generating solution: %s success", cmake_path.path);
	} else {
		ERR("generating solution: %s failed", cmake_path.path);
	}

	hashmap_iterate_c(&sln->projects, gen_proj_make, &gen_proj_make_data);

	return ret;
}