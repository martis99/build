#include "sln.h"

#include "dir.h"
#include "proj.h"
#include "prop.h"

#include "md5.h"

#include "defines.h"
#include "mem.h"
#include "utils.h"

#include <Windows.h>
#include <stdio.h>
#include <string.h>

#define SLN_CMAKE_USER_FOLDERS_SHIFT 0

#define SLN_CMAKE_USER_FOLDERS (1 << SLN_CMAKE_USER_FOLDERS_SHIFT)

#define CMAKE_VERSION_MAJOR 3
#define CMAKE_VERSION_MINOR 16

static const prop_pol_t s_sln_props[] = {
	[SLN_PROP_NAME]		 = { .name = "NAME", .parse = prop_parse_word },
	[SLN_PROP_LANGS]	 = { .name = "LANGS", .parse = prop_parse_langs, .print = prop_print_langs },
	[SLN_PROP_DIRS]		 = { .name = "DIRS", .parse = prop_parse_path, .dim = PROP_DIM_ARRAY },
	[SLN_PROP_STARTUP]	 = { .name = "STARTUP", .parse = prop_parse_word },
	[SLN_PROP_CONFIGS]	 = { .name = "CONFIGS", .parse = prop_parse_word, .dim = PROP_DIM_ARRAY },
	[SLN_PROP_PLATFORMS] = { .name = "PLATFORMS", .parse = prop_parse_word, .dim = PROP_DIM_ARRAY },
	[SLN_PROP_CHARSET]	 = { .name = "CHARSET", .parse = prop_parse_word, .str_table = s_charsets, .str_table_len = __CHARSET_MAX },
	[SLN_PROP_OUTDIR]	 = { .name = "OUTDIR", .parse = prop_parse_path },
	[SLN_PROP_INTDIR]	 = { .name = "INTDIR", .parse = prop_parse_path },

};

typedef struct read_dir_data_s {
	sln_t *sln;
	dir_t *parent;
} read_dir_data_t;

static int read_dir(path_t *path, const char *folder, void *usr)
{
	int ret = 0;

	MSG("entering directory: %s", path->path);

	path_t file_path = *path;
	if (path_child(&file_path, "Project.txt", 11)) {
		return 1;
	}

	read_dir_data_t *data = usr;
	if (file_exists(file_path.path)) {
		proj_t *proj = m_calloc(1, sizeof(proj_t));
		ret += proj_read(proj, &data->sln->path, path, data->parent);
		prop_str_t *name = &proj->props[PROJ_PROP_NAME].value;
		if (hashmap_get(&data->sln->projects, name->data, name->len, NULL)) {
			ERR_LOGICS("Project '%.*s' with the same name already exists", name->path, name->line + 1, name->start - name->line_start + 1, name->len, name->data);
			m_free(proj, sizeof(proj_t));
		} else {
			hashmap_set(&data->sln->projects, proj->props[PROJ_PROP_NAME].value.data, proj->props[PROJ_PROP_NAME].value.len, proj);
		}

		return ret;
	}

	path_set_len(&file_path, path->len);
	if (path_child(&file_path, "Directory.txt", 13)) {
		return 1;
	}

	dir_t *dir = m_calloc(1, sizeof(dir_t));
	if (hashmap_get(&data->sln->dirs, path->path, path->len, NULL)) {
		//ERR_LOGICS("Direcotry '%.*s' with the same path already exists", name->path, name->line + 1, name->start - name->line_start + 1, name->len, name->data);
		m_free(dir, sizeof(dir_t));
	} else {
		hashmap_set(&data->sln->dirs, path->path, path->len, dir);
		ret += dir_read(dir, &data->sln->path, path, read_dir, data->parent, data);
	}

	return ret;
}

static int str_cmp_data(const prop_str_t **arr_val, const prop_str_t **value)
{
	return (*arr_val)->len == (*value)->len && memcmp((*arr_val)->data, (*value)->data, (*value)->len) == 0;
}

static void get_all_depends(array_t *arr, proj_t *proj, hashmap_t *projects)
{
	array_t *depends = &proj->props[PROJ_PROP_DEPENDS].arr;
	for (int i = 0; i < depends->count; i++) {
		prop_str_t *depend = array_get(depends, i);

		if (array_index_cb(arr, &depend, str_cmp_data) == -1) {
			array_add(arr, &depend);
		}

		proj_t *dep_proj = NULL;
		if (hashmap_get(projects, depend->data, depend->len, &dep_proj)) {
			get_all_depends(arr, dep_proj, projects);
		}
	}
}

static void calculate_depends(void *key, size_t ksize, void *value, void *usr)
{
	sln_t *sln	 = usr;
	proj_t *proj = value;

	if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE) {
		array_t *depends = &proj->props[PROJ_PROP_DEPENDS].arr;
		array_init(&proj->all_depends, depends->capacity * 2, sizeof(prop_str_t *));
		get_all_depends(&proj->all_depends, proj, &sln->projects);
	}
}

int sln_read(sln_t *sln, const path_t *path)
{
	sln->path	   = *path;
	sln->file_path = *path;

	if (path_child(&sln->file_path, "Solution.txt", 12)) {
		return 1;
	}

	if ((sln->data.len = (unsigned int)file_read(sln->file_path.path, 1, sln->file, DATA_LEN)) == -1) {
		return 1;
	}

	sln->data.path = sln->file_path.path;
	sln->data.data = sln->file;
	sln->data.cur  = 0;

	int ret = props_parse_file(&sln->data, sln->props, s_sln_props, sizeof(s_sln_props));

	path_t name = { 0 };
	path_init(&name, sln->props[SLN_PROP_NAME].value.data, sln->props[SLN_PROP_NAME].value.len);
	path_child_s(&name, "sln", 3, '.');

	unsigned char buf[256] = { 0 };
	md5(name.path, name.len, buf, sizeof(buf), sln->guid, sizeof(sln->guid));

	array_t *dirs				= &sln->props[SLN_PROP_DIRS].arr;
	path_t child_path			= sln->path;
	unsigned int child_path_len = child_path.len;

	hashmap_create(&sln->projects, 16);
	hashmap_create(&sln->dirs, 16);

	read_dir_data_t read_dir_data = {
		.sln	= sln,
		.parent = NULL,
	};

	for (int i = 0; i < dirs->count; i++) {
		prop_str_t *dir = array_get(dirs, i);
		path_child(&child_path, dir->data, dir->len);
		if (folder_exists(child_path.path)) {
			if (read_dir(&child_path, dir->data, &read_dir_data)) {
				ret += 1;
				continue;
			}
		} else {
			ERR_LOGICS("Folder '%.*s' doesn't exists", dir->path, dir->line + 1, dir->start - dir->line_start + 1, dir->len, dir->data);
			ret += 1;
		}
		child_path.len = child_path_len;
	}

	hashmap_iterate(&sln->projects, calculate_depends, sln);

	return ret;
}

static void print_project(void *key, size_t ksize, void *value, void *usr)
{
	proj_print(value);
}

static void print_dir(void *key, size_t ksize, void *value, void *usr)
{
	dir_print(value);
}

void sln_print(sln_t *sln)
{
	INFP("Solution\n"
		 "    Path: %.*s\n"
		 "    File: %.*s\n"
		 "    GUID: %s",
		 sln->path.len, sln->path.path, sln->file_path.len, sln->file_path.path, sln->guid);

	props_print(sln->props, s_sln_props, sizeof(s_sln_props));

	hashmap_iterate(&sln->dirs, print_dir, NULL);
	hashmap_iterate(&sln->projects, print_project, NULL);
}

static void gen_dir_cmake(void *key, size_t ksize, void *value, const void *usr)
{
	dir_gen_cmake(value, usr);
}

typedef struct gen_proj_cmake_data_s {
	const path_t *path;
	const hashmap_t *projects;
	int lang;
	charset_t charset;
} gen_proj_cmake_data_t;

static void gen_proj_cmake(void *key, size_t ksize, void *value, const void *usr)
{
	const gen_proj_cmake_data_t *data = usr;
	proj_gen_cmake(value, data->projects, data->path, data->lang, data->charset);
}

int sln_gen_cmake(const sln_t *sln, const path_t *path)
{
	if (!folder_exists(path->path)) {
		ERR("folder does not exists: %.*s", (unsigned int)path->len, path->path);
		return 1;
	}

	const prop_str_t *name	   = &sln->props[SLN_PROP_NAME].value;
	unsigned int languages	   = sln->props[SLN_PROP_LANGS].mask;
	unsigned int properties	   = SLN_CMAKE_USER_FOLDERS;
	const char *targets_folder = "CMake";
	const array_t *dirs		   = &sln->props[SLN_PROP_DIRS].arr;
	const prop_str_t *startup  = &sln->props[SLN_PROP_STARTUP].value;

	path_t cmake_path = *path;
	if (path_child(&cmake_path, "CMakeLists.txt", 14)) {
		return 1;
	}

	FILE *fp = file_open(cmake_path.path, "w", 1);
	if (fp == NULL) {
		return 1;
	}

	MSG("generating solution: %s", cmake_path.path);

	fprintf_s(fp, "cmake_minimum_required(VERSION %d.%d)\n\nproject(\"%.*s\" LANGUAGES", CMAKE_VERSION_MAJOR, CMAKE_VERSION_MINOR, name->len, name->data);

	const char *langs[] = {
		[LANG_SHIFT_NONE] = "",
		[LANG_SHIFT_C]	  = " C",
		[LANG_SHIFT_ASM]  = " ASM",
	};

	for (int i = 0; i < __LANG_SHIFT_MAX; i++) {
		if (languages & (1 << i)) {
			fprintf_s(fp, langs[i]);
		}
	}
	fprintf_s(fp, ")\n");

	if (properties & SLN_CMAKE_USER_FOLDERS) {
		fprintf_s(fp, "\nset_property(GLOBAL PROPERTY USE_FOLDERS ON)\n");
		if (targets_folder) {
			fprintf_s(fp, "set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER %s)\n", targets_folder);
		}
	}

	fprintf_s(fp, "\n");

	int ret = 0;

	for (int i = 0; i < dirs->count; i++) {
		prop_str_t *dir = array_get(dirs, i);
		fprintf_s(fp, "add_subdirectory(%.*s)\n", dir->len, dir->data);
	}

	if (hashmap_get(&sln->projects, startup->data, startup->len, NULL)) {
		fprintf_s(fp, "\nset_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT %.*s)\n", startup->len, startup->data);
	} else {
		ERR_LOGICS("project '%.*s' doesn't exists", startup->path, startup->line + 1, startup->start - startup->line_start + 1, startup->len, startup->data);
		ret = 1;
	}

	fclose(fp);
	if (ret == 0) {
		SUC("generating solution: %s success", cmake_path.path);
	} else {
		ERR("generating solution: %s failed", cmake_path.path);
	}

	hashmap_iterate_c(&sln->dirs, gen_dir_cmake, path);

	gen_proj_cmake_data_t gen_proj_cmake_data = {
		.path	  = path,
		.projects = &sln->projects,
		.lang	  = languages,
		.charset  = sln->props[SLN_PROP_CHARSET].mask,
	};
	hashmap_iterate_c(&sln->projects, gen_proj_cmake, &gen_proj_cmake_data);

	return ret;
}

typedef struct gen_proj_make_data_s {
	const path_t *path;
	const hashmap_t *projects;
	int lang;
	charset_t charset;
} gen_proj_make_data_t;

static void gen_proj_make(void *key, size_t ksize, void *value, const void *usr)
{
	const gen_proj_make_data_t *data = usr;
	proj_gen_make(value, data->projects, data->path);
}

static void add_phony_make(void *key, size_t ksize, void *value, void *usr)
{
	proj_t *proj = value;
	fprintf_s(usr, " %.*s", proj->props[PROJ_PROP_NAME].value.len, proj->props[PROJ_PROP_NAME].value.data);
}

static void convert_slash(char *dst, const char *src, size_t len)
{
	memcpy(dst, src, len);
	for (int i = 0; i < len; i++) {
		if (dst[i] == '\\') {
			dst[i] = '/';
		}
	}
}

static void add_target_make(void *key, size_t ksize, void *value, void *usr)
{
	proj_t *proj	   = value;
	char buf[MAX_PATH] = { 0 };
	convert_slash(buf, proj->rel_path.path, proj->rel_path.len);
	fprintf_s(usr,
			  "%.*s:\n"
			  "\t$(MAKE) -C %.*s SLNDIR=$(SLNDIR)\n"
			  "\n",
			  proj->props[PROJ_PROP_NAME].value.len, proj->props[PROJ_PROP_NAME].value.data, proj->rel_path.len, buf);
}

int sln_gen_make(const sln_t *sln, const path_t *path)
{
	if (!folder_exists(path->path)) {
		ERR("folder does not exists: %.*s", (unsigned int)path->len, path->path);
		return 1;
	}

	const prop_str_t *name	   = &sln->props[SLN_PROP_NAME].value;
	unsigned int languages	   = sln->props[SLN_PROP_LANGS].mask;
	unsigned int properties	   = SLN_CMAKE_USER_FOLDERS;
	const char *targets_folder = "CMake";
	const array_t *dirs		   = &sln->props[SLN_PROP_DIRS].arr;
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

static void add_dir_sln_vs(void *key, size_t ksize, void *value, void *usr)
{
	FILE *fp		= usr;
	dir_t *dir		= value;
	pathv_t *folder = &dir->folder;

	fprintf_s(fp, "Project(\"{2150E333-8FDC-42A3-9474-1A3956D46DE8}\") = \"%.*s\", \"%.*s\", \"{%s}\"\nEndProject\n", (unsigned int)folder->len, folder->path,
			  (unsigned int)folder->len, folder->path, dir->guid);
}

static void add_proj_sln_vs(void *key, size_t ksize, void *value, void *usr)
{
	FILE *fp		 = usr;
	proj_t *proj	 = value;
	prop_str_t *name = &proj->props[PROJ_PROP_NAME].value;
	fprintf_s(fp, "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"%.*s\", \"%.*s\\%.*s.vcxproj\", \"{%s}\"\nEndProject\n", (unsigned int)name->len, name->data,
			  (unsigned int)proj->rel_path.len, proj->rel_path.path, (unsigned int)name->len, name->data, proj->guid);
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
	FILE *fp	 = usr;
	proj_t *proj = value;

	if (proj->parent) {
		fprintf_s(fp, "\t\t{%s} = {%s}\n", proj->guid, proj->parent->guid);
	}
}

typedef struct proj_config_plat_vs_s {
	const array_t *configs;
	const array_t *platforms;
	FILE *fp;
} proj_config_plat_vs_t;

static void proj_config_plat_vs(void *key, size_t ksize, void *value, void *usr)
{
	proj_config_plat_vs_t *data = usr;
	proj_t *proj				= value;

	for (int i = 0; i < data->configs->count; i++) {
		prop_str_t *config = array_get(data->configs, i);
		for (int j = 0; j < data->platforms->count; j++) {
			prop_str_t *platform   = array_get(data->platforms, j);
			const char *platf	   = platform->data;
			unsigned int platf_len = platform->len;
			if (platform->len == 3 && strncmp(platform->data, "x86", 3) == 0) {
				platf	  = "Win32";
				platf_len = 5;
			}

			fprintf_s(data->fp,
					  "\t\t{%s}.%.*s|%.*s.ActiveCfg = %.*s|%.*s\n"
					  "\t\t{%s}.%.*s|%.*s.Build.0 = %.*s|%.*s\n",
					  proj->guid, config->len, config->data, platform->len, platform->data, config->len, config->data, platf_len, platf, proj->guid, config->len,
					  config->data, platform->len, platform->data, config->len, config->data, platf_len, platf);
		}
	}
}

typedef struct gen_proj_vs_data_s {
	const path_t *path;
	const hashmap_t *projects;
	const array_t *configs;
	const array_t *platforms;
	const prop_t *charset;
	const prop_t *outdir;
	const prop_t *intdir;
} gen_proj_vs_data_t;

static void gen_proj_vs(void *key, size_t ksize, void *value, const void *usr)
{
	const gen_proj_vs_data_t *data = usr;
	proj_gen_vs(value, data->projects, data->path, data->configs, data->platforms, data->charset, data->outdir, data->intdir);
}

int sln_gen_vs(const sln_t *sln, const path_t *path)
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
				  "VisualStudioVersion = 17.3.32819.101\n"
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
		.fp		   = fp,
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
		.charset   = &sln->props[SLN_PROP_CHARSET],
		.outdir	   = &sln->props[SLN_PROP_OUTDIR],
		.intdir	   = &sln->props[SLN_PROP_INTDIR],
	};
	hashmap_iterate_c(&sln->projects, gen_proj_vs, &gen_proj_vs_data);

	return 0;
}

static void free_project(void *key, size_t ksize, void *value, void *usr)
{
	proj_free(value);
	m_free(value, sizeof(proj_t));
}

static void free_dir(void *key, size_t ksize, void *value, void *usr)
{
	dir_free(value);
	m_free(value, sizeof(dir_t));
}

void sln_free(sln_t *sln)
{
	hashmap_iterate(&sln->projects, free_project, sln);
	hashmap_free(&sln->projects);
	hashmap_iterate(&sln->dirs, free_dir, sln);
	hashmap_free(&sln->dirs);
	props_free(sln->props, s_sln_props, sizeof(s_sln_props));
}
