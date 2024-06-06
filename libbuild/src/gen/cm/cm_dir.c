#include "cm_dir.h"

#include "common.h"

#include "cstr.h"

int cm_dir_gen(const dir_t *dir, const path_t *path)
{
	const arr_t *dirs = &dir->props[DIR_PROP_DIRS].arr;

	path_t cmake_path = *path;
	if (path_child(&cmake_path, dir->rel_dir.path, dir->rel_dir.len) == NULL) {
		return 1;
	}

	if (!folder_exists(cmake_path.path)) {
		folder_create(cmake_path.path);
	}

	if (path_child(&cmake_path, CSTR("CMakeLists.txt")) == NULL) {
		return 1;
	}

	FILE *file = file_open(cmake_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	MSG("generating directories: %s", cmake_path.path);

	int ret = 0;

	for (uint i = 0; i < dirs->cnt; i++) {
		prop_str_t *dir = arr_get(dirs, i);
		if (dir->val.data == NULL) {
			continue;
		}

		c_fprintf(file, "add_subdirectory(%.*s)\n", (int)dir->val.len - 1, dir->val.data);
	}

	file_close(file);

	if (ret == 0) {
		SUC("generating directories: %s success", cmake_path.path);
	} else {
		ERR("generating directories: %s failed", cmake_path.path);
	}
	return ret;
}
