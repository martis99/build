#include "vs_sln.h"

#include "vs_proj.h"

#include "gen/dir.h"

#include "defines.h"
#include "utils.h"

static void add_dir_sln_vs(void *key, size_t ksize, void *value, void *usr)
{
	FILE *fp	= usr;
	dir_t *dir	= value;
	pathv_t *folder = &dir->folder;

	fprintf_s(fp, "Project(\"{2150E333-8FDC-42A3-9474-1A3956D46DE8}\") = \"%.*s\", \"%.*s\", \"{%s}\"\nEndProject\n", (unsigned int)folder->len, folder->path,
		  (unsigned int)folder->len, folder->path, dir->guid);
}

static void add_proj_sln_vs(void *key, size_t ksize, void *value, void *usr)
{
	FILE *fp	       = usr;
	proj_t *proj	       = value;
	const prop_str_t *name = proj->name;
	fprintf_s(fp, "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"%.*s\", \"%.*s\\%.*s.vcxproj\", \"{%s}\"\nEndProject\n", (unsigned int)name->len,
		  name->data, (unsigned int)proj->rel_path.len, proj->rel_path.path, (unsigned int)name->len, name->data, proj->guid);
}

typedef struct proj_config_plat_vs_s {
	const array_t *configs;
	const array_t *platforms;
	FILE *fp;
} proj_config_plat_vs_t;

static void proj_config_plat_vs(void *key, size_t ksize, void *value, void *usr)
{
	proj_config_plat_vs_t *data = usr;
	proj_t *proj		    = value;

	for (int i = 0; i < data->configs->count; i++) {
		prop_str_t *config = array_get(data->configs, i);
		for (int j = 0; j < data->platforms->count; j++) {
			prop_str_t *platform   = array_get(data->platforms, j);
			const char *platf      = platform->data;
			unsigned int platf_len = platform->len;
			if (platform->len == 3 && strncmp(platform->data, "x86", 3) == 0) {
				platf	  = "Win32";
				platf_len = 5;
			}

			fprintf_s(data->fp,
				  "\t\t{%s}.%.*s|%.*s.ActiveCfg = %.*s|%.*s\n"
				  "\t\t{%s}.%.*s|%.*s.Build.0 = %.*s|%.*s\n",
				  proj->guid, config->len, config->data, platform->len, platform->data, config->len, config->data, platf_len, platf, proj->guid,
				  config->len, config->data, platform->len, platform->data, config->len, config->data, platf_len, platf);
		}
	}
}

static void add_dir_nested_vs(void *key, size_t ksize, void *value, void *usr)
{
	FILE *fp   = usr;
	dir_t *dir = value;

	if (dir->parent) {
		fprintf_s(fp, "\t\t{%s} = {%s}\n", dir->guid, dir->parent->guid);
	}
}

static void add_proj_nested_vs(void *key, size_t ksize, void *value, void *usr)
{
	FILE *fp     = usr;
	proj_t *proj = value;

	if (proj->parent) {
		fprintf_s(fp, "\t\t{%s} = {%s}\n", proj->guid, proj->parent->guid);
	}
}

typedef struct gen_proj_vs_data_s {
	const path_t *path;
	const hashmap_t *projects;
	const array_t *configs;
	const array_t *platforms;
	const prop_t *langs;
	const prop_t *charset;
	const prop_t *outdir;
	const prop_t *intdir;
} gen_proj_vs_data_t;

static void gen_proj_vs(void *key, size_t ksize, void *value, const void *usr)
{
	const gen_proj_vs_data_t *data = usr;
	vs_proj_gen(value, data->projects, data->path, data->configs, data->platforms, data->langs, data->charset, data->outdir, data->intdir);
}

int vs_sln_gen(const sln_t *sln, const path_t *path)
{
	if (!folder_exists(path->path)) {
		ERR("folder does not exists: %.*s", path->len, path->path);
		return 1;
	}

	path_t cmake_path = *path;
	if (path_child(&cmake_path, sln->props[SLN_PROP_NAME].value.data, sln->props[SLN_PROP_NAME].value.len)) {
		return 1;
	}

	if (path_child_s(&cmake_path, "sln", 3, '.')) {
		return 1;
	}

	FILE *fp = file_open(cmake_path.path, "w", 1);
	if (fp == NULL) {
		return 1;
	}

	MSG("generating solution: %s", cmake_path.path);

	int ret = 0;

	fprintf_s(fp, "Microsoft Visual Studio Solution File, Format Version 12.00\n"
		      "# Visual Studio Version 17\n"
		      "VisualStudioVersion = 17.4.33205.214\n"
		      "MinimumVisualStudioVersion = 10.0.40219.1\n");

	hashmap_iterate_hc(&sln->dirs, add_dir_sln_vs, fp);
	hashmap_iterate_hc(&sln->projects, add_proj_sln_vs, fp);

	fprintf_s(fp, "Global\n"
		      "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n");

	if (!sln->props[SLN_PROP_CONFIGS].set || !sln->props[SLN_PROP_PLATFORMS].set || sln->props[SLN_PROP_CONFIGS].arr.count < 1 ||
	    sln->props[SLN_PROP_PLATFORMS].arr.count < 1) {
		ERR("at least one config and platform must be set");
		return ret + 1;
	}

	const array_t *configs	 = &sln->props[SLN_PROP_CONFIGS].arr;
	const array_t *platforms = &sln->props[SLN_PROP_PLATFORMS].arr;

	for (int i = 0; i < configs->count; i++) {
		prop_str_t *config = array_get(configs, i);
		for (int j = 0; j < platforms->count; j++) {
			prop_str_t *platform = array_get(platforms, j);
			fprintf_s(fp, "\t\t%.*s|%.*s = %.*s|%.*s\n", config->len, config->data, platform->len, platform->data, config->len, config->data, platform->len,
				  platform->data);
		}
	}

	fprintf_s(fp, "\tEndGlobalSection\n"
		      "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n");

	proj_config_plat_vs_t proj_config_plat_vs_data = {
		.configs   = configs,
		.platforms = platforms,
		.fp	   = fp,
	};

	hashmap_iterate_hc(&sln->projects, proj_config_plat_vs, &proj_config_plat_vs_data);

	fprintf_s(fp, "\tEndGlobalSection\n"
		      "\tGlobalSection(SolutionProperties) = preSolution\n"
		      "\t\tHideSolutionNode = FALSE\n"
		      "\tEndGlobalSection\n"
		      "\tGlobalSection(NestedProjects) = preSolution\n");

	hashmap_iterate_hc(&sln->dirs, add_dir_nested_vs, fp);
	hashmap_iterate_hc(&sln->projects, add_proj_nested_vs, fp);

	fprintf_s(fp,
		  "\tEndGlobalSection\n"
		  "\tGlobalSection(ExtensibilityGlobals) = postSolution\n"
		  "\t\tSolutionGuid = {%s}\n"
		  "\tEndGlobalSection\n"
		  "EndGlobal\n",
		  sln->guid);

	fclose(fp);
	if (ret == 0) {
		SUC("generating solution: %s success", cmake_path.path);
	} else {
		ERR("generating solution: %s failed", cmake_path.path);
	}

	gen_proj_vs_data_t gen_proj_vs_data = {
		.path	   = path,
		.projects  = &sln->projects,
		.configs   = &sln->props[SLN_PROP_CONFIGS].arr,
		.platforms = &sln->props[SLN_PROP_PLATFORMS].arr,
		.langs	   = &sln->props[SLN_PROP_LANGS],
		.charset   = &sln->props[SLN_PROP_CHARSET],
		.outdir	   = &sln->props[SLN_PROP_OUTDIR],
		.intdir	   = &sln->props[SLN_PROP_INTDIR],
	};
	hashmap_iterate_c(&sln->projects, gen_proj_vs, &gen_proj_vs_data);

	return 0;
}
