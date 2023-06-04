#include "vs_sln.h"

#include "vs_proj.h"

#include "gen/dir.h"

#include "common.h"

static void add_dir_sln_vs(void *key, size_t ksize, void *value, void *priv)
{
	FILE *fp	= priv;
	dir_t *dir	= value;
	pathv_t *folder = &dir->folder;

	p_fprintf(fp, "Project(\"{2150E333-8FDC-42A3-9474-1A3956D46DE8}\") = \"%.*s\", \"%.*s\", \"{%s}\"\nEndProject\n", (int)folder->len, folder->path,
		  (int)folder->len, folder->path, dir->guid);
}

static void add_proj_sln_vs(void *key, size_t ksize, void *value, void *priv)
{
	FILE *fp	       = priv;
	proj_t *proj	       = value;
	const prop_str_t *name = proj->name;
	p_fprintf(fp, "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"%.*s\", \"%.*s\\%.*s.vcxproj\", \"{%s}\"\nEndProject\n", (int)name->len, name->data,
		  (int)proj->rel_path.len, proj->rel_path.path, (int)name->len, name->data, proj->guid);
}

typedef struct proj_config_plat_vs_s {
	const prop_t *configs;
	const arr_t *platforms;
	FILE *file;
} proj_config_plat_vs_t;

static void proj_config_plat_vs(void *key, size_t ksize, void *value, void *priv)
{
	proj_config_plat_vs_t *data = priv;
	proj_t *proj		    = value;

	for (uint i = 0; i < data->configs->arr.cnt; i++) {
		prop_str_t *config = arr_get(&data->configs->arr, i);
		for (uint j = 0; j < data->platforms->cnt; j++) {
			prop_str_t *platform = arr_get(data->platforms, j);
			const char *platf    = platform->data;
			size_t platf_len     = platform->len;
			if (cstr_cmp(platform->data, platform->len, "x86", 3)) {
				platf	  = "Win32";
				platf_len = 5;
			}

			p_fprintf(data->file,
				  "\t\t{%s}.%.*s|%.*s.ActiveCfg = %.*s|%.*s\n"
				  "\t\t{%s}.%.*s|%.*s.Build.0 = %.*s|%.*s\n",
				  proj->guid, config->len, config->data, platform->len, platform->data, config->len, config->data, platf_len, platf, proj->guid,
				  config->len, config->data, platform->len, platform->data, config->len, config->data, platf_len, platf);
		}
	}
}

static void add_dir_nested_vs(void *key, size_t ksize, void *value, void *priv)
{
	FILE *fp   = priv;
	dir_t *dir = value;

	if (dir->parent) {
		p_fprintf(fp, "\t\t{%s} = {%s}\n", dir->guid, dir->parent->guid);
	}
}

static void add_proj_nested_vs(void *key, size_t ksize, void *value, void *priv)
{
	FILE *fp     = priv;
	proj_t *proj = value;

	if (proj->parent) {
		p_fprintf(fp, "\t\t{%s} = {%s}\n", proj->guid, proj->parent->guid);
	}
}

typedef struct gen_proj_vs_data_s {
	const path_t *path;
	const hashmap_t *projects;
	const prop_t *sln_props;
} gen_proj_vs_data_t;

static void gen_proj_vs(void *key, size_t ksize, void *value, const void *priv)
{
	const gen_proj_vs_data_t *data = priv;
	vs_proj_gen(value, data->projects, data->path, data->sln_props);
}

int vs_sln_gen(const sln_t *sln, const path_t *path)
{
	if (!folder_exists(path->path)) {
		ERR("folder does not exists: %.*s", (int)path->len, path->path);
		return 1;
	}

	path_t cmake_path = *path;
	if (path_child(&cmake_path, sln->props[SLN_PROP_NAME].value.data, sln->props[SLN_PROP_NAME].value.len)) {
		return 1;
	}

	if (path_child_s(&cmake_path, "sln", 3, '.')) {
		return 1;
	}

	FILE *file = file_open(cmake_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	MSG("generating solution: %s", cmake_path.path);

	int ret = 0;

	p_fprintf(file, "Microsoft Visual Studio Solution File, Format Version 12.00\n"
			"# Visual Studio Version 17\n"
			"VisualStudioVersion = 17.4.33205.214\n"
			"MinimumVisualStudioVersion = 10.0.40219.1\n");

	hashmap_iterate_hc(&sln->dirs, add_dir_sln_vs, file);
	hashmap_iterate_hc(&sln->projects, add_proj_sln_vs, file);

	p_fprintf(file, "Global\n"
			"\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n");

	if (!(sln->props[SLN_PROP_CONFIGS].flags & PROP_SET) || !(sln->props[SLN_PROP_PLATFORMS].flags & PROP_SET) || sln->props[SLN_PROP_CONFIGS].arr.cnt < 1 ||
	    sln->props[SLN_PROP_PLATFORMS].arr.cnt < 1) {
		ERR("%s", "at least one config and platform must be set");
		return ret + 1;
	}

	const prop_t *configs  = &sln->props[SLN_PROP_CONFIGS];
	const arr_t *platforms = &sln->props[SLN_PROP_PLATFORMS].arr;

	for (uint i = 0; i < configs->arr.cnt; i++) {
		prop_str_t *config = arr_get(&configs->arr, i);
		for (uint j = 0; j < platforms->cnt; j++) {
			prop_str_t *platform = arr_get(platforms, j);
			p_fprintf(file, "\t\t%.*s|%.*s = %.*s|%.*s\n", config->len, config->data, platform->len, platform->data, config->len, config->data, platform->len,
				  platform->data);
		}
	}

	p_fprintf(file, "\tEndGlobalSection\n"
			"\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n");

	proj_config_plat_vs_t proj_config_plat_vs_data = {
		.configs   = configs,
		.platforms = platforms,
		.file	   = file,
	};

	hashmap_iterate_hc(&sln->projects, proj_config_plat_vs, &proj_config_plat_vs_data);

	p_fprintf(file, "\tEndGlobalSection\n"
			"\tGlobalSection(SolutionProperties) = preSolution\n"
			"\t\tHideSolutionNode = FALSE\n"
			"\tEndGlobalSection\n"
			"\tGlobalSection(NestedProjects) = preSolution\n");

	hashmap_iterate_hc(&sln->dirs, add_dir_nested_vs, file);
	hashmap_iterate_hc(&sln->projects, add_proj_nested_vs, file);

	p_fprintf(file,
		  "\tEndGlobalSection\n"
		  "\tGlobalSection(ExtensibilityGlobals) = postSolution\n"
		  "\t\tSolutionGuid = {%s}\n"
		  "\tEndGlobalSection\n"
		  "EndGlobal\n",
		  sln->guid);

	file_close(file);
	if (ret == 0) {
		SUC("generating solution: %s success", cmake_path.path);
	} else {
		ERR("generating solution: %s failed", cmake_path.path);
	}

	gen_proj_vs_data_t gen_proj_vs_data = {
		.path	   = path,
		.projects  = &sln->projects,
		.sln_props = sln->props,
	};

	hashmap_iterate_c(&sln->projects, gen_proj_vs, &gen_proj_vs_data);

	return 0;
}
