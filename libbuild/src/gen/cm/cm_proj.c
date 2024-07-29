#include "cm_proj.h"

#include "common.h"

str_t *cm_proj_get_vars(const proj_t *proj, str_t *vars)
{
	vars[PROJ_VAR_SLNDIR]	= STR("${CMAKE_SOURCE_DIR}/");
	vars[PROJ_VAR_PROJDIR]	= strc(proj->rel_dir.path, proj->rel_dir.len);
	vars[PROJ_VAR_PROJNAME] = strc(proj->name.data, proj->name.len);
	vars[PROJ_VAR_CONFIG]	= STR("${CMAKE_BUILD_TYPE}");
	vars[PROJ_VAR_ARCH]	= STR("${ARCH}");
	return vars;
}

int cm_proj_gen(proj_t *proj, const dict_t *projects, const prop_t *sln_props)
{
	path_t gen_path = { 0 };
	if (path_init(&gen_path, proj->dir.path, proj->dir.len) == NULL) {
		return 1;
	}

	if (!folder_exists(gen_path.path)) {
		folder_create(gen_path.path);
	}

	if (path_child(&gen_path, CSTR("CMakeLists.txt")) == NULL) {
		return 1;
	}

	log_info("build", "proj", NULL, "generating project: %s", gen_path.path);

	FILE *file = file_open(gen_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	int len = cmake_print(&proj->gen.cmake, PRINT_DST_FILE(file));

	file_close(file);

	if (len == 0) {
		log_error("build", "proj", NULL, "failed to generate project: %s", gen_path.path);
		return 1;
	}

	return 0;
}

void cm_proj_free(proj_t *proj)
{
	cmake_free(&proj->gen.cmake);
}
