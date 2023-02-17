#include "cm_dir.h"

#include "defines.h"
#include "print.h"
#include "utils.h"

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

	if (path_child(&cmake_path, "CMakeLists.txt", 14)) {
		return 1;
	}

	FILE *fp = file_open(cmake_path.path, "w", 1);
	if (fp == NULL) {
		return 1;
	}

	MSG("generating directories: %s", cmake_path.path);

	int ret = 0;

	for (int i = 0; i < dirs->count; i++) {
		prop_str_t *dir = array_get(dirs, i);
		p_fprintf(fp, "add_subdirectory(%.*s)\n", dir->len, dir->data);
	}

	fclose(fp);

	if (ret == 0) {
		SUC("generating directories: %s success", cmake_path.path);
	} else {
		ERR("generating directories: %s failed", cmake_path.path);
	}
	return ret;
}
