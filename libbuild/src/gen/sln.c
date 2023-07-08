#include "gen/sln.h"

#include "dir.h"
#include "gen/proj.h"

#include "common.h"

#include "c_md5.h"

static const prop_pol_t s_sln_props[] = {
	[SLN_PROP_NAME]	     = { .name = STR("NAME"), },
	[SLN_PROP_LANGS]     = { .name = STR("LANGS"),  .str_table = s_langs, .str_table_len = __LANG_MAX, .arr = 1 },
	[SLN_PROP_DIRS]	     = { .name = STR("DIRS"),  .arr = 1 },
	[SLN_PROP_STARTUP]   = { .name = STR("STARTUP"), },
	[SLN_PROP_CONFIGS]   = { .name = STR("CONFIGS"),.arr = 1 },
	[SLN_PROP_PLATFORMS] = { .name = STR("PLATFORMS"), .arr = 1 },
	[SLN_PROP_CHARSET]   = { .name = STR("CHARSET"), .str_table = s_charsets, .str_table_len = __CHARSET_MAX },
	[SLN_PROP_CFLAGS]    = { .name = STR("CFLAGS"), .str_table = s_cflags, .str_table_len = __CFLAG_MAX, .arr = 1 },
	[SLN_PROP_OUTDIR]    = { .name = STR("OUTDIR"), .def = STR("$(SLN_DIR)\\bin\\$(CONFIG)-$(PLATFORM)\\$(PROJ_FOLDER)\\")},
	[SLN_PROP_INTDIR]    = { .name = STR("INTDIR"), .def = STR("$(SLN_DIR)\\bin\\$(CONFIG)-$(PLATFORM)\\$(PROJ_FOLDER)\\int\\")},
};

typedef struct read_dir_data_s {
	sln_t *sln;
	dir_t *parent;
} read_dir_data_t;

static int read_dir(path_t *path, const char *folder, void *priv)
{
	int ret = 0;

	MSG("entering directory: %s", path->path);

	path_t file_path = *path;
	if (path_child(&file_path, CSTR("Project.txt"))) {
		return 1;
	}

	read_dir_data_t *data = priv;
	if (file_exists(file_path.path)) {
		proj_t *proj = m_calloc(1, sizeof(proj_t));
		ret += proj_read(proj, &data->sln->path, path, data->parent, data->sln->props);
		const prop_str_t *name = proj->name;
		if (hashmap_get(&data->sln->projects, name->data, name->len, NULL)) {
			hashmap_set(&data->sln->projects, name->data, name->len, proj);
		} else {
			proj_free(proj);
			m_free(proj, sizeof(proj_t));
			return -1;
		}

		return ret;
	}

	path_set_len(&file_path, path->len);
	if (path_child(&file_path, CSTR("Directory.txt"))) {
		return 1;
	}

	dir_t *dir = m_calloc(1, sizeof(dir_t));
	if (hashmap_get(&data->sln->dirs, path->path, path->len, NULL)) {
		hashmap_set(&data->sln->dirs, path->path, path->len, dir);
		ret += dir_read(dir, &data->sln->path, path, read_dir, data->parent, data);
	} else {
		//ERR_LOGICS("Direcotry '%.*s' with the same path already exists", name->path, name->line + 1, name->start - name->line_start + 1, name->len, name->data);
		m_free(dir, sizeof(dir_t));
	}

	return ret;
}

static int proj_cmp_name(const void *proj, const void *name)
{
	return prop_cmp((*(const proj_t **)proj)->name, *(const prop_str_t **)name);
}

static void get_all_depends(arr_t *arr, proj_t *proj, hashmap_t *projects)
{
	arr_t *depends = &proj->props[PROJ_PROP_DEPENDS].arr;
	for (uint i = 0; i < depends->cnt; i++) {
		prop_str_t *dname = arr_get(depends, i);

		proj_t *dproj = NULL;
		if (hashmap_get(projects, dname->data, dname->len, (void **)&dproj)) {
			ERR("project doesn't exists: '%.*s'", (int)dname->len, dname->data);
			continue;
		}

		get_all_depends(arr, dproj, projects);

		if (arr_index_cmp(arr, &dname, proj_cmp_name) == -1) {
			arr_app(arr, &dproj);
		}
	}
}

static void calculate_depends(void *key, size_t ksize, void *value, void *priv)
{
	sln_t *sln   = priv;
	proj_t *proj = value;

	if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE || proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_BIN) {
		arr_t *depends = &proj->props[PROJ_PROP_DEPENDS].arr;
		arr_init(&proj->all_depends, depends->cap * 2, sizeof(proj_t *));
		get_all_depends(&proj->all_depends, proj, &sln->projects);

		for (uint i = 0; i < proj->all_depends.cnt; i++) {
			const proj_t *dproj = *(proj_t **)arr_get(&proj->all_depends, i);

			if (!(dproj->props[PROJ_PROP_DEPEND].flags & PROP_SET)) {
				continue;
			}

			const arr_t *depend = &dproj->props[PROJ_PROP_DEPEND].arr;

			for (uint i = 0; i < depend->cnt; i++) {
				prop_str_t *dpname = arr_get(depend, i);

				proj_t *dpproj = NULL;
				if (hashmap_get(&sln->projects, dpname->data, dpname->len, (void **)&dpproj)) {
					ERR("project doesn't exists: '%.*s'", (int)dpname->len, dpname->data);
					continue;
				}

				if (arr_index_cmp(&proj->all_depends, &dpname, proj_cmp_name) == -1) {
					arr_app(&proj->all_depends, &dpproj);
				}
			}
		}
	}
}

static void get_all_includes(arr_t *arr, proj_t *proj, hashmap_t *projects)
{
	arr_t *includes = &proj->props[PROJ_PROP_INCLUDES].arr;
	for (uint i = 0; i < includes->cnt; i++) {
		prop_str_t *iname = arr_get(includes, i);

		proj_t *iproj = NULL;
		if (hashmap_get(projects, iname->data, iname->len, (void **)&iproj)) {
			ERR("project doesn't exists: '%.*s'", (int)iname->len, iname->data);
			continue;
		}

		get_all_includes(arr, iproj, projects);

		if (arr_index_cmp(arr, &iname, proj_cmp_name) == -1) {
			arr_app(arr, &iproj);
		}
	}
}

static void calculate_includes(void *key, size_t ksize, void *value, void *priv)
{
	sln_t *sln   = priv;
	proj_t *proj = value;

	arr_t *includes = &proj->props[PROJ_PROP_INCLUDES].arr;
	arr_init(&proj->includes, includes->cap * 2, sizeof(proj_t *));
	get_all_includes(&proj->includes, proj, &sln->projects);
}

int sln_read(sln_t *sln, const path_t *path)
{
	sln->path      = *path;
	sln->file_path = *path;

	if (path_child(&sln->file_path, CSTR("Solution.txt"))) {
		return 1;
	}

	if (!file_exists(sln->file_path.path)) {
		ERR_INPUT("file doesn't exists: '%.*s'", __func__, (int)sln->file_path.len, sln->file_path.path);
		return 1;
	}

	if ((sln->data.len = file_read_t(sln->file_path.path, sln->file, DATA_LEN)) == -1) {
		return 1;
	}

	sln->data.path = sln->file_path.path;
	sln->data.data = sln->file;

	int ret = props_parse_file(sln->data, sln->props, s_sln_props, sizeof(s_sln_props));

	path_t name = { 0 };
	path_init(&name, sln->props[SLN_PROP_NAME].value.data, sln->props[SLN_PROP_NAME].value.len);
	path_child_s(&name, CSTR("sln"), '.');

	byte buf[256] = { 0 };
	c_md5(name.path, name.len, buf, sizeof(buf), sln->guid, sizeof(sln->guid));

	arr_t *dirs	      = &sln->props[SLN_PROP_DIRS].arr;
	path_t child_path     = sln->path;
	size_t child_path_len = child_path.len;

	hashmap_create(&sln->projects, 16);
	hashmap_create(&sln->dirs, 16);

	read_dir_data_t read_dir_data = {
		.sln	= sln,
		.parent = NULL,
	};

	prop_def(sln->props, s_sln_props, sizeof(s_sln_props));

	for (uint i = 0; i < dirs->cnt; i++) {
		prop_str_t *dir = arr_get(dirs, i);
		path_child(&child_path, dir->data, dir->len);
		if (folder_exists(child_path.path)) {
			const int r = read_dir(&child_path, dir->data, &read_dir_data);
			if (r == -1) {
				dir->data = NULL;
			} else {
				ret += r;
			}
		} else {
			ERR_LOGICS("Folder '%.*s' doesn't exists", dir->path, dir->line, dir->col, (int)dir->len, dir->data);
			ret += 1;
		}
		child_path.len = child_path_len;
	}

	hashmap_iterate(&sln->projects, calculate_depends, sln);
	hashmap_iterate(&sln->projects, calculate_includes, sln);

	return ret;
}

static void print_project(void *key, size_t ksize, void *value, void *priv)
{
	proj_print(value);
}

static void print_dir(void *key, size_t ksize, void *value, void *priv)
{
	dir_print(value);
}

void sln_print(sln_t *sln)
{
	INFP("Solution\n"
	     "    Path: %.*s\n"
	     "    File: %.*s\n"
	     "    GUID: %s",
	     (int)sln->path.len, sln->path.path, (int)sln->file_path.len, sln->file_path.path, sln->guid);

	props_print(sln->props, s_sln_props, sizeof(s_sln_props));

	hashmap_iterate(&sln->dirs, print_dir, NULL);
	hashmap_iterate(&sln->projects, print_project, NULL);
}

static void free_project(void *key, size_t ksize, void *value, void *priv)
{
	proj_free(value);
	m_free(value, sizeof(proj_t));
}

static void free_dir(void *key, size_t ksize, void *value, void *priv)
{
	dir_free(value);
	m_free(value, sizeof(dir_t));
}

void sln_free(sln_t *sln)
{
	hashmap_iterate(&sln->projects, free_project, sln);
	hashmap_free(&sln->projects);
	hashmap_iterate(&sln->dirs, free_dir, sln);
	hashmap_free(&sln->dirs);
	props_free(sln->props, s_sln_props, sizeof(s_sln_props));
}
