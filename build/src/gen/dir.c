#include "dir.h"

#include "proj.h"

#include "common.h"

#include "c_md5.h"

static const prop_pol_t s_dir_props[] = {
	[DIR_PROP_DIRS] = { .name = STR("DIRS"), .arr = 1 },
};

static int add_dir(path_t *path, const char *folder, void *priv)
{
	size_t folder_len = cstr_len(folder);

	prop_str_t dir = {
		.path = NULL,
		.data = m_calloc(folder_len + 1, sizeof(char)),
		.len  = folder_len,
	};

	m_memcpy(dir.data, dir.len, folder, folder_len);
	array_add(priv, &dir);
	return 0;
}

typedef struct read_dir_data_s {
	void *sln;
	dir_t *parent;
} read_dir_data_t;

int dir_read(dir_t *dir, const path_t *sln_path, const path_t *path, on_dir_cb on_dir, const dir_t *parent, void *priv)
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
		if ((dir->data.len = file_read_t(dir->file_path.path, dir->file, DATA_LEN)) == -1) {
			return 1;
		}

		dir->data.path = dir->file_path.path;
		dir->data.data = dir->file;

		ret += props_parse_file(dir->data, dir->props, s_dir_props, sizeof(s_dir_props));

	} else {
		array_init(&dir->props[DIR_PROP_DIRS].arr, 8, sizeof(prop_str_t));
		dir->props[DIR_PROP_DIRS].flags |= PROP_ARR;
		ret += files_foreach(path, add_dir, NULL, &dir->props[DIR_PROP_DIRS].arr);
	}

	byte buf[256] = { 0 };
	c_md5(dir->dir.path, dir->dir.len, buf, sizeof(buf), dir->guid, sizeof(dir->guid));

	array_t *subdirs = &dir->props[DIR_PROP_DIRS].arr;

	path_t child_path     = *path;
	size_t child_path_len = child_path.len;

	read_dir_data_t *data	      = priv;
	read_dir_data_t read_dir_data = {
		.sln	= data->sln,
		.parent = dir,
	};

	for (int i = 0; i < subdirs->count; i++) {
		prop_str_t *dir = array_get(subdirs, i);
		path_child(&child_path, dir->data, dir->len);
		if (folder_exists(child_path.path)) {
			if (on_dir(&child_path, dir->data, &read_dir_data)) {
				ret	       = 1;
				child_path.len = child_path_len;
				continue;
			}
		} else {
			ERR_LOGICS("Folder '%.*s' doesn't exists", dir->path, dir->line, dir->col, (int)dir->len, dir->data);
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
	     (int)dir->path.len, dir->path.path, (int)dir->file_path.len, dir->file_path.path, (int)dir->dir.len, dir->dir.path, (int)dir->folder.len, dir->folder.path,
	     dir->guid);

	if (dir->parent) {
		INFP("    Parent : %.*s", (int)dir->parent->folder.len, dir->parent->folder.path);
	} else {
		INFP("%s", "    Parent :");
	}
	props_print(dir->props, s_dir_props, sizeof(s_dir_props));
}

static void free_dir(int index, void *value, void *priv)
{
	prop_str_t *dir = value;
	m_free(dir->data, (size_t)dir->len + 1);
}

void dir_free(dir_t *dir)
{
	if (!(dir->props[DIR_PROP_DIRS].flags & PROP_SET)) {
		array_iterate(&dir->props[DIR_PROP_DIRS].arr, free_dir, NULL);
	}
	props_free(dir->props, s_dir_props, sizeof(s_dir_props));
}
