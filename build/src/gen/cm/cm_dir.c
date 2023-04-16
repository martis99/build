#include "cm_dir.h"

#include "common.h"

int cm_dir_gen(const dir_t *dir, const path_t *path)
{
	const array_t *dirs = &dir->props[DIR_PROP_DIRS].arr;

	path_t cmake_path = *path;
	if (path_child(&cmake_path, dir->dir.path, dir->dir.len)) {
		return 1;
	}

	if (!folder_exists(cmake_path.path)) {
		folder_create(cmake_path.path);
	}

	if (path_child(&cmake_path, CSTR("CMakeLists.txt"))) {
		return 1;
	}

	FILE *file = file_open(cmake_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	MSG("generating directories: %s", cmake_path.path);

	int ret = 0;

	for (int i = 0; i < dirs->count; i++) {
		prop_str_t *dir = array_get(dirs, i);
		if (dir->data == NULL) {
			continue;
		}

		p_fprintf(file, "add_subdirectory(%.*s)\n", (int)dir->len, dir->data);
	}

	file_close(file);

	if (ret == 0) {
		SUC("generating directories: %s success", cmake_path.path);
	} else {
		ERR("generating directories: %s failed", cmake_path.path);
	}
	return ret;
}
