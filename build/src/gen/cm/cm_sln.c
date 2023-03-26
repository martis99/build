#include "cm_sln.h"

#include "cm_dir.h"
#include "cm_proj.h"

#include "common.h"

#define SLN_CMAKE_USER_FOLDERS_SHIFT 0

#define SLN_CMAKE_USER_FOLDERS (1 << SLN_CMAKE_USER_FOLDERS_SHIFT)

#define CMAKE_VERSION_MAJOR 3
#define CMAKE_VERSION_MINOR 16

static void gen_dir_cmake(void *key, size_t ksize, void *value, const void *priv)
{
	cm_dir_gen(value, priv);
}

typedef struct gen_proj_cmake_data_s {
	const path_t *path;
	const hashmap_t *projects;
	const prop_t *sln_props;
} gen_proj_cmake_data_t;

static void gen_proj_cmake(void *key, size_t ksize, void *value, const void *priv)
{
	const gen_proj_cmake_data_t *data = priv;
	cm_proj_gen(value, data->projects, data->path, data->sln_props);
}

int cm_sln_gen(const sln_t *sln, const path_t *path)
{
	if (!folder_exists(path->path)) {
		ERR("folder does not exists: %.*s", (unsigned int)path->len, path->path);
		return 1;
	}

	const prop_str_t *name	   = &sln->props[SLN_PROP_NAME].value;
	unsigned int languages	   = sln->props[SLN_PROP_LANGS].mask;
	unsigned int properties	   = SLN_CMAKE_USER_FOLDERS;
	const char *targets_folder = "CMake";
	const array_t *dirs	   = &sln->props[SLN_PROP_DIRS].arr;
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

	p_fprintf(fp, "cmake_minimum_required(VERSION %d.%d)\n\nproject(\"%.*s\" LANGUAGES", CMAKE_VERSION_MAJOR, CMAKE_VERSION_MINOR, name->len, name->data);

	// clang-format off
	const char *langs[] = {
		[LANG_NONE]    = "",
		[LANG_C]       = " C",
		[LANG_ASM]     = " ASM",
		[LANG_CPP]     = " CXX",
	};
	// clang-format on

	for (int i = 0; i < __LANG_MAX; i++) {
		if (languages & (1 << i)) {
			p_fprintf(fp, langs[i]);
		}
	}
	p_fprintf(fp, ")\n");

	if (sln->props[SLN_PROP_CONFIGS].flags & PROP_SET) {
		p_fprintf(fp, "\nset(CMAKE_CONFIGURATION_TYPES \"");

		const array_t *configs = &sln->props[SLN_PROP_CONFIGS].arr;
		int first	       = 0;
		for (int i = 0; i < configs->count; i++) {
			prop_str_t *config = array_get(configs, i);
			p_fprintf(fp, "%.*s%.*s", first, ";", config->len, config->data);
			first = 1;
		}

		p_fprintf(fp, "\" CACHE STRING \"\" FORCE)\n");
	}

	if (properties & SLN_CMAKE_USER_FOLDERS) {
		p_fprintf(fp, "\nset_property(GLOBAL PROPERTY USE_FOLDERS ON)\n");
		if (targets_folder) {
			p_fprintf(fp, "set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER %s)\n", targets_folder);
		}
	}

	p_fprintf(fp, "\n");

	int ret = 0;

	for (int i = 0; i < dirs->count; i++) {
		prop_str_t *dir = array_get(dirs, i);
		p_fprintf(fp, "add_subdirectory(%.*s)\n", dir->len, dir->data);
	}

	if (hashmap_get(&sln->projects, startup->data, startup->len, NULL)) {
		ERR_LOGICS("project '%.*s' doesn't exists", startup->path, startup->line, startup->col, startup->len, startup->data);
		ret = 1;
	} else {
		p_fprintf(fp, "\nset_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT %.*s)\n", startup->len, startup->data);
	}

	fclose(fp);
	if (ret == 0) {
		SUC("generating solution: %s success", cmake_path.path);
	} else {
		ERR("generating solution: %s failed", cmake_path.path);
	}

	hashmap_iterate_c(&sln->dirs, gen_dir_cmake, path);

	gen_proj_cmake_data_t gen_proj_cmake_data = {
		.path	   = path,
		.projects  = &sln->projects,
		.sln_props = sln->props,
	};
	hashmap_iterate_c(&sln->projects, gen_proj_cmake, &gen_proj_cmake_data);

	return ret;
}
