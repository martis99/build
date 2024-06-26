#ifndef DIR_H
#define DIR_H

#include "build.h"
#include "path.h"
#include "prop.h"
#include "str.h"

#include <stdio.h>

typedef enum dir_prop_e {
	DIR_PROP_DIRS,

	__DIR_PROP_MAX,
} dir_prop_t;

typedef struct dir_s {
	path_t path;
	pathv_t dir;
	pathv_t rel_dir;
	str_t name;
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

typedef int (*on_dir_cb)(path_t *path, const char *folder, void *priv);
int dir_read(build_t *build, dir_t *dir, const pathv_t *sln_path, const path_t *path, on_dir_cb on_dir, const dir_t *parent, void *priv);
void dir_print(dir_t *dir);

void dir_free(dir_t *dir);

#endif
