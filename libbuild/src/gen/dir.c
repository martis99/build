#include "dir.h"

#include "gen/proj.h"

#include "common.h"

#include "md5.h"

static const prop_pol_t s_dir_props[] = {
	[DIR_PROP_DIRS] = { .name = STRS("DIRS"), .arr = 1 },
};

static int add_dir(path_t *path, const char *folder, void *priv)
{
	size_t folder_len = cstr_len(folder);

	prop_str_t dir = {
		.path = NULL,
		.val  = strn(folder, folder_len, folder_len + 1),
	};

	arr_app(priv, &dir);
	return 0;
}

typedef struct read_dir_data_s {
	build_t *build;
	void *sln;
	dir_t *parent;
} read_dir_data_t;

int dir_read(build_t *build, dir_t *dir, const path_t *sln_path, const path_t *path, on_dir_cb on_dir, const dir_t *parent, void *priv)
{
	dir->file_path = *path;
	pathv_path(&dir->path, &dir->file_path);
	pathv_sub(&dir->dir, &dir->file_path, sln_path);
	pathv_folder(&dir->folder, &dir->file_path);

	dir->parent = parent;

	if (path_child(&dir->file_path, CSTR("Directory.txt")) == NULL) {
		return 1;
	}

	int ret = 0;

	if (file_exists(dir->file_path.path)) {
		if ((dir->data.val.len = file_read_t(dir->file_path.path, dir->file, DATA_LEN)) == -1) {
			return 1;
		}

		dir->data.path = dir->file_path.path;
		dir->data.val  = strb(dir->file, sizeof(dir->file), dir->data.val.len);

		ret += props_parse_file(dir->data, &build->ini_prs, dir->props, s_dir_props, sizeof(s_dir_props));

	} else {
		arr_init(&dir->props[DIR_PROP_DIRS].arr, 8, sizeof(prop_str_t));
		dir->props[DIR_PROP_DIRS].flags |= PROP_SET | PROP_ARR;
		ret += files_foreach(path, add_dir, NULL, &dir->props[DIR_PROP_DIRS].arr);
	}

	byte buf[256] = { 0 };
	md5(dir->dir.path, dir->dir.len, buf, sizeof(buf), dir->guid, sizeof(dir->guid));

	arr_t *subdirs = &dir->props[DIR_PROP_DIRS].arr;

	path_t child_path     = *path;
	size_t child_path_len = child_path.len;

	read_dir_data_t *data	      = priv;
	read_dir_data_t read_dir_data = {
		.build	= build,
		.sln	= data->sln,
		.parent = dir,
	};

	for (uint i = 0; i < subdirs->cnt; i++) {
		prop_str_t *dir = arr_get(subdirs, i);
		path_child(&child_path, dir->val.data, dir->val.len);
		if (folder_exists(child_path.path)) {
			int r = on_dir(&child_path, dir->val.data, &read_dir_data);
			if (r == -1) {
				str_free(&dir->val);
			} else if (r) {
				ret	       = 1;
				child_path.len = child_path_len;
				continue;
			}
		} else {
			ERR_LOGICS("Folder '%.*s' doesn't exists", dir->path, dir->line, dir->col, (int)dir->val.len, dir->val.data);
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

void dir_free(dir_t *dir)
{
	if (!(dir->props[DIR_PROP_DIRS].flags & PROP_SET)) {
		prop_str_t *prop = NULL;
		arr_foreach(&dir->props[DIR_PROP_DIRS].arr, prop)
		{
			if (prop->val.data == NULL) {
				continue;
			}

			str_free(&prop->val);
		}
	}
	props_free(dir->props, s_dir_props, sizeof(s_dir_props));
}
