#ifndef DIR_H
#define DIR_H

#include "types.h"
#include "utils_types.h"

#include <stdio.h>

typedef enum dir_prop_e {
	DIR_PROP_DIRS,

	__DIR_PROP_MAX,
} dir_prop_t;

typedef struct dir_s {
	pathv_t path;
	path_t file_path;
	pathv_t dir;
	pathv_t folder;
	char file[1024];
	prop_str_t data;
	prop_t props[__DIR_PROP_MAX];
	char guid[37];
	const struct dir_s *parent;
} dir_t;

typedef struct dir_data_s {
	void *sln;
	FILE *cmake;
} dir_data_t;

typedef int(*on_dir_cb)(path_t *path, const char *folder, void *usr);
int dir_read(dir_t *dir, const path_t *sln_path, const path_t *path, on_dir_cb on_dir, const dir_t *parent, void *usr);
void dir_print(dir_t *dir);

int dir_gen_cmake(const dir_t *dir, const path_t *path);

void dir_free(dir_t *dir);

#endif