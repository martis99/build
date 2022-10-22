#include "dir.h"

#include "proj.h"
#include "prop.h"

#include "md5.h"
#include "utils.h"

#include "defines.h"
#include "mem.h"

#include <Windows.h>
#include <stdio.h>
#include <string.h>

static const prop_pol_t s_dir_props[] = {
	[DIR_PROP_DIRS] = { .name = "DIRS", .parse = prop_parse_path, .dim = PROP_DIM_ARRAY },
};

static int add_dir(path_t *path, const char *folder, void *usr)
{
	unsigned int folder_len = cstr_len(folder);

	prop_str_t dir = {
		.path		= NULL,
		.data		= m_calloc(folder_len + 1, sizeof(char)),
		.start		= 0,
		.len		= folder_len,
		.line		= 0,
		.line_start = 0,
	};

	memcpy(dir.data, folder, dir.len);
	array_add(usr, &dir);
	return 0;
}

typedef struct read_dir_data_s {
	void *sln;
	dir_t *parent;
} read_dir_data_t;

int dir_read(dir_t *dir, const path_t *sln_path, const path_t *path, on_dir_cb on_dir, const dir_t *parent, void *usr)
{
	dir->file_path = *path;
	pathv_path(&dir->path, &dir->file_path);
	pathv_sub(&dir->dir, &dir->file_path, sln_path);
	pathv_folder(&dir->folder, &dir->file_path);

	dir->parent = parent;

	if (path_child(&dir->file_path, "Directory.txt", 13)) {
		return 1;
	}

	int ret = 0;

	if (file_exists(dir->file_path.path)) {
		if ((dir->data.len = (unsigned int)file_read(dir->file_path.path, 1, dir->file, DATA_LEN)) == -1) {
			return 1;
		}

		dir->data.path = dir->file_path.path;
		dir->data.data = dir->file;
		dir->data.cur  = 0;

		ret += props_parse_file(&dir->data, dir->props, s_dir_props, sizeof(s_dir_props));

	} else {
		array_init(&dir->props[DIR_PROP_DIRS].arr, 8, sizeof(prop_str_t));
		ret += files_foreach(path, add_dir, NULL, &dir->props[DIR_PROP_DIRS].arr);
	}

	unsigned char buf[256] = { 0 };
	md5(dir->dir.path, dir->dir.len, buf, sizeof(buf), dir->guid, sizeof(dir->guid));

	array_t *subdirs = &dir->props[DIR_PROP_DIRS].arr;

	path_t child_path			= *path;
	unsigned int child_path_len = child_path.len;

	read_dir_data_t *data		  = usr;
	read_dir_data_t read_dir_data = {
		.sln	= data->sln,
		.parent = dir,
	};

	for (int i = 0; i < subdirs->count; i++) {
		prop_str_t *dir = array_get(subdirs, i);
		path_child(&child_path, dir->data, dir->len);
		if (folder_exists(child_path.path)) {
			if (on_dir(&child_path, dir->data, &read_dir_data)) {
				ret = 1;
				continue;
			}
		} else {
			ERR_LOGICS("Folder '%.*s' doesn't exists", dir->path, dir->line + 1, dir->start - dir->line_start + 1, dir->len, dir->data);
			ret = 1;
		}
		child_path.len = child_path_len;
	}

	return ret;
}

void dir_print(dir_t *dir)
{
	INFP("Directory\n"
		 "    Path   : %.*s\n"
		 "    File   : %.*s\n"
		 "    Dir    : %.*s\n"
		 "    Folder : %.*s\n"
		 "    GUID   : %s",
		 dir->path.len, dir->path.path, dir->file_path.len, dir->file_path.path, dir->dir.len, dir->dir.path, dir->folder.len, dir->folder.path, dir->guid);

	if (dir->parent) {
		INFP("    Parent : %.*s", (unsigned int)dir->parent->folder.len, dir->parent->folder.path);
	} else {
		INFP("    Parent :");
	}
	props_print(dir->props, s_dir_props, sizeof(s_dir_props));
}

int dir_gen_cmake(const dir_t *dir, const path_t *path)
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

	path_t child_path	  = *path;
	size_t child_path_len = child_path.len;

	for (int i = 0; i < dirs->count; i++) {
		prop_str_t *dir = array_get(dirs, i);
		fprintf_s(fp, "add_subdirectory(%.*s)\n", dir->len, dir->data);
	}

	fclose(fp);

	if (ret == 0) {
		SUC("generating directories: %s success", cmake_path.path);
	} else {
		ERR("generating directories: %s failed", cmake_path.path);
	}
	return ret;
}

static void free_dir(int index, void *value, void *usr)
{
	prop_str_t *dir = value;
	m_free(dir->data, (size_t)dir->len + 1);
}

void dir_free(dir_t *dir)
{
	if (!dir->props[DIR_PROP_DIRS].set) {
		array_iterate(&dir->props[DIR_PROP_DIRS].arr, free_dir, NULL);
	}
	props_free(dir->props, s_dir_props, sizeof(s_dir_props));
}
