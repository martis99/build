#include "dir.h"

#include "gen/proj.h"

#include "common.h"

#include "md5.h"

static const prop_pol_t s_dir_props[] = {
	[DIR_PROP_DIRS] = { .name = STRS("DIRS"), .flags = PPF_ARR | PPF_DIR },
};

static int add_dir(path_t *path, const char *folder, void *priv)
{
	size_t folder_len = cstr_len(folder);

	prop_str_t dir = {
		.path = NULL,
		.val  = strn(folder, folder_len, folder_len + 2),
	};

	((char *)dir.val.data)[dir.val.len++] = '/';
	((char *)dir.val.data)[dir.val.len]   = '\0';

	arr_app(priv, &dir);
	return 0;
}

typedef struct read_dir_data_s {
	build_t *build;
	void *sln;
	dir_t *parent;
} read_dir_data_t;

int dir_read(build_t *build, dir_t *dir, const pathv_t *sln_dir, const path_t *path, on_dir_cb on_dir, const dir_t *parent, void *priv)
{
	dir->path    = *path;
	dir->dir     = pathv_get_dir(pathv_path(&dir->path), NULL);
	dir->rel_dir = (pathv_t){
		.path = dir->dir.path + sln_dir->len,
		.len  = dir->dir.len - sln_dir->len,
	};
	pathv_get_dir(dir->dir, &dir->name);

	dir->parent = parent;

	int ret = 0;

	if (file_exists(dir->path.path)) {
		if ((dir->data.val.len = file_read_t(dir->path.path, dir->file, DATA_LEN)) == -1) {
			return 1;
		}

		dir->data.path = dir->path.path;
		dir->data.val  = strb(dir->file, sizeof(dir->file), dir->data.val.len);

		ret += props_parse_file(dir->data, &build->ini_prs, dir->props, s_dir_props, sizeof(s_dir_props));

	} else {
		arr_init(&dir->props[DIR_PROP_DIRS].arr, 8, sizeof(prop_str_t));
		dir->props[DIR_PROP_DIRS].flags |= PROP_SET | PROP_ARR;
		path_t pdir = { 0 };
		path_init(&pdir, dir->dir.path, dir->dir.len);
		ret += files_foreach(&pdir, add_dir, NULL, &dir->props[DIR_PROP_DIRS].arr);
	}

	byte buf[256] = { 0 };
	md5(dir->dir.path, dir->dir.len, buf, sizeof(buf), dir->guid, sizeof(dir->guid));

	arr_t *subdirs = &dir->props[DIR_PROP_DIRS].arr;

	path_t child_path = { 0 };
	path_init(&child_path, dir->dir.path, dir->dir.len);
	size_t child_path_len = child_path.len;

	read_dir_data_t *data	      = priv;
	read_dir_data_t read_dir_data = {
		.build	= build,
		.sln	= data->sln,
		.parent = dir,
	};

	for (uint i = 0; i < subdirs->cnt; i++) {
		prop_str_t *dir = arr_get(subdirs, i);
		path_child_dir(&child_path, dir->val.data, dir->val.len);
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
			ERR_LOGICS("Folder '%.*s' doesn't exists", dir->path, dir->line, dir->col, (int)child_path.len, child_path.path);
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
	     "    Dir    : %.*s\n"
	     "    Rel    : %.*s\n"
	     "    Name   : %.*s\n"
	     "    GUID   : %s",
	     (int)dir->path.len, dir->path.path, (int)dir->dir.len, dir->dir.path, (int)dir->rel_dir.len, dir->rel_dir.path, (int)dir->name.len, dir->name.data,
	     dir->guid);

	if (dir->parent) {
		INFP("    Parent : %.*s", (int)dir->parent->dir.len, dir->parent->dir.path);
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
