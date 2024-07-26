#include "gen/cm/cm_sln.h"

#include "cm_dir.h"
#include "cm_proj.h"
#include "gen/cm/pgc_gen_cm.h"
#include "gen/proj_gen.h"

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

	c_fprintf(file, "if(ARCH STREQUAL \"x86_64\")\n"
			"\tset(CMAKE_ASM_NASM_OBJECT_FORMAT \"elf64\")\n"
			"\tset(CMAKE_ASM_FLAGS \"-m64\")\n"
			"\tset(CMAKE_C_FLAGS \"-m64\")\n"
			"\tset(CMAKE_CXX_FLAGS \"-m64\")\n"
			"else()\n"
			"\tset(CMAKE_ASM_NASM_OBJECT_FORMAT \"elf32\")\n"
			"\tset(CMAKE_ASM_FLAGS \"-m32\")\n"
			"\tset(CMAKE_C_FLAGS \"-m32\")\n"
			"\tset(CMAKE_CXX_FLAGS \"-m32\")\n"
			"endif()\n\n");

	c_fprintf(
		file,
		"set(CMAKE_ASM_CREATE_SHARED_LIBRARY \"<CMAKE_ASM_COMPILER> <CMAKE_SHARED_LIBRARY_ASM_FLAGS> ${CMAKE_ASM_FLAGS} <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_ASM_FLAGS> <SONAME_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>\")\n");
	c_fprintf(
		file,
		"set(CMAKE_ASM_NASM_LINK_EXECUTABLE \"<CMAKE_ASM_COMPILER> <FLAGS> ${CMAKE_ASM_FLAGS} <CMAKE_ASM_NASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>\")\n");

	c_fprintf(file, "\nproject(\"%.*s\" LANGUAGES", name->val.len, name->val.data);

	// clang-format off
	const char *langs[] = {
		[LANG_NONE]    = "",
		[LANG_NASM]    = " ASM_NASM",
		[LANG_ASM]     = " ASM",
		[LANG_C]       = " C",
		[LANG_CPP]     = " CXX",
	};
	// clang-format on

	for (int i = 0; i < __LANG_MAX; i++) {
		c_fprintf(file, langs[i]);
	}
	c_fprintf(file, ")\n\n");

	if (sln->props[SLN_PROP_CONFIGS].flags & PROP_SET) {
		c_fprintf(file, "set(CMAKE_CONFIGURATION_TYPES \"");

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

	str_t vars[__PROJ_VAR_MAX] = { 0 };

	const proj_t **pproj;
	arr_foreach(&sln->build_order, pproj)
	{
		proj_t *proj = *(proj_t **)pproj;

		proj_gen(proj, &sln->projects, sln->props, &proj->pgc);

		cm_proj_get_vars(proj, vars);
		pgc_replace_vars(&proj->pgc, &proj->pgcr, s_proj_vars, vars, __PROJ_VAR_MAX);

		cmake_init(&proj->gen.cmake, 16, 8, 16);
		pgc_gen_cm(&proj->pgc, &proj->gen.cmake);

		ret |= cm_proj_gen(proj, &sln->projects, sln->props);
	}

	return ret;
}

void cm_sln_free(sln_t *sln)
{
	dict_foreach(&sln->projects, pair)
	{
		cm_proj_free(pair->value);
	}
}
