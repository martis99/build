#include "gen/cm/cm_sln.h"

#include "cm_dir.h"
#include "cm_proj.h"

#include "common.h"

#define SLN_CMAKE_USER_FOLDERS_SHIFT 0

#define SLN_CMAKE_USER_FOLDERS (1 << SLN_CMAKE_USER_FOLDERS_SHIFT)

#define CMAKE_VERSION_MAJOR 3
#define CMAKE_VERSION_MINOR 16

int cm_sln_gen(sln_t *sln, const path_t *path)
{
	if (!folder_exists(path->path)) {
		ERR("folder does not exists: %.*s", (int)path->len, path->path);
		return 1;
	}

	const prop_str_t *name	   = &sln->props[SLN_PROP_NAME].value;
	uint languages		   = sln->props[SLN_PROP_LANGS].mask;
	uint properties		   = SLN_CMAKE_USER_FOLDERS;
	const char *targets_folder = "CMake";
	const arr_t *dirs	   = &sln->props[SLN_PROP_DIRS].arr;
	const prop_str_t *startup  = &sln->props[SLN_PROP_STARTUP].value;

	path_t cmake_path = *path;
	if (path_child(&cmake_path, CSTR("CMakeLists.txt")) == NULL) {
		return 1;
	}

	FILE *file = file_open(cmake_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	MSG("generating solution: %s", cmake_path.path);

	c_fprintf(file, "cmake_minimum_required(VERSION %d.%d)\n\n", CMAKE_VERSION_MAJOR, CMAKE_VERSION_MINOR);

	/*c_fprintf(file, "if(NOT CMAKE_NASM_COMPILER)\n"
			"  set(CMAKE_NASM_COMPILER \"nasm\")\n"
			"endif()\n\n");*/

	c_fprintf(file, "project(\"%.*s\" LANGUAGES", name->val.len, name->val.data);

	// clang-format off
	const char *langs[] = {
		[LANG_NONE]    = "",
		[LANG_NASM]    = " NASM",
		[LANG_ASM]     = " ASM",
		[LANG_C]       = " C",
		[LANG_CPP]     = " CXX",
	};
	// clang-format on

	for (int i = 0; i < __LANG_MAX; i++) {
		if (i != LANG_NASM) {
			c_fprintf(file, langs[i]);
		}
	}
	c_fprintf(file, ")\n");

	c_fprintf(
		file,
		"if(NOT CMAKE_ASM_CREATE_SHARED_LIBRARY)\n"
		"  set(CMAKE_ASM_CREATE_SHARED_LIBRARY\n"
		"      \"<CMAKE_LINKER> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS_<CONFIG>> <CMAKE_SHARED_LIBRARY_LINK_C_FLAGS> <CMAKE_SHARED_LIBRARY_LINK_C_FLAGS_<CONFIG>> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG> <CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_<CONFIG>> <LINK_LIBRARIES> <CMAKE_ASM_OUTPUT_FILE_FLAG><TARGET> <OBJECTS> <CMAKE_ASM_LINK_FLAGS> <LINK_FLAGS> <LINK_PATH> <LINK_LIBRARIES>\")\n"
		"endif()\n\n");

	if (sln->props[SLN_PROP_CONFIGS].flags & PROP_SET) {
		c_fprintf(file, "\nset(CMAKE_CONFIGURATION_TYPES \"");

		const arr_t *configs = &sln->props[SLN_PROP_CONFIGS].arr;
		int first	     = 0;
		for (uint i = 0; i < configs->cnt; i++) {
			prop_str_t *config = arr_get(configs, i);
			c_fprintf(file, "%.*s%.*s", first, ";", config->val.len, config->val.data);
			first = 1;
		}

		c_fprintf(file, "\" CACHE STRING \"\" FORCE)\n");
	}

	if (properties & SLN_CMAKE_USER_FOLDERS) {
		c_fprintf(file, "\nset_property(GLOBAL PROPERTY USE_FOLDERS ON)\n");
		if (targets_folder) {
			c_fprintf(file, "set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER %s)\n", targets_folder);
		}
	}

	c_fprintf(file, "\n");

	int ret = 0;

	for (uint i = 0; i < dirs->cnt; i++) {
		prop_str_t *dir = arr_get(dirs, i);
		if (dir->val.data == NULL) {
			continue;
		}

		c_fprintf(file, "add_subdirectory(%.*s)\n", dir->val.len - 1, dir->val.data);
	}

	if (sln->props[SLN_PROP_STARTUP].flags & PROP_SET) {
		if (dict_get(&sln->projects, startup->val.data, startup->val.len, NULL)) {
			ERR_LOGICS("project '%.*s' doesn't exists", startup->path, startup->line, startup->col, (int)startup->val.len, startup->val.data);
			ret = 1;
		} else {
			c_fprintf(file, "\nset_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT %.*s)\n", startup->val.len, startup->val.data);
		}
	}

	file_close(file);
	if (ret == 0) {
		SUC("generating solution: %s success", cmake_path.path);
	} else {
		ERR("generating solution: %s failed", cmake_path.path);
	}

	dict_foreach(&sln->dirs, pair)
	{
		ret |= cm_dir_gen(pair->value, path);
	}

	dict_foreach(&sln->projects, pair)
	{
		ret |= cm_proj_gen(pair->value, &sln->projects, sln->props);
	}

	return ret;
}
