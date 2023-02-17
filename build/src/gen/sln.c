#include "sln.h"

#include "dir.h"
#include "proj.h"
#include "prop.h"

#include "md5.h"

#include "defines.h"
#include "mem.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

static const prop_pol_t s_sln_props[] = {
	[SLN_PROP_NAME]	     = { .name = "NAME", .parse = prop_parse_word },
	[SLN_PROP_LANGS]     = { .name = "LANGS", .parse = prop_parse_word, .str_table = s_langs, .str_table_len = __LANG_MAX, .dim = PROP_DIM_ARRAY },
	[SLN_PROP_DIRS]	     = { .name = "DIRS", .parse = prop_parse_path, .dim = PROP_DIM_ARRAY },
	[SLN_PROP_STARTUP]   = { .name = "STARTUP", .parse = prop_parse_word },
	[SLN_PROP_CONFIGS]   = { .name = "CONFIGS", .parse = prop_parse_word, .dim = PROP_DIM_ARRAY },
	[SLN_PROP_PLATFORMS] = { .name = "PLATFORMS", .parse = prop_parse_word, .dim = PROP_DIM_ARRAY },
	[SLN_PROP_CHARSET]   = { .name = "CHARSET", .parse = prop_parse_word, .str_table = s_charsets, .str_table_len = __CHARSET_MAX },
	[SLN_PROP_CFLAGS]    = { .name = "CFLAGS", .parse = prop_parse_word, .str_table = s_cflags, .str_table_len = __CFLAG_MAX, .dim = PROP_DIM_ARRAY },
	[SLN_PROP_OUTDIR]    = { .name = "OUTDIR", .parse = prop_parse_path },
	[SLN_PROP_INTDIR]    = { .name = "INTDIR", .parse = prop_parse_path },

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
	if (path_child(&file_path, "Project.txt", 11)) {
		return 1;
	}

	read_dir_data_t *data = priv;
	if (file_exists(file_path.path)) {
		proj_t *proj = m_calloc(1, sizeof(proj_t));
		ret += proj_read(proj, &data->sln->path, path, data->parent);
		const prop_str_t *name = proj->name;
		if (hashmap_get(&data->sln->projects, name->data, name->len, NULL)) {
			hashmap_set(&data->sln->projects, name->data, name->len, proj);
		} else {
			ERR_LOGICS("Project '%.*s' with the same name already exists", name->path, name->line + 1, name->start - name->line_start + 1, name->len,
				   name->data);
			m_free(proj, sizeof(proj_t));
		}

		return ret;
	}

	path_set_len(&file_path, path->len);
	if (path_child(&file_path, "Directory.txt", 13)) {
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

static void get_all_depends(array_t *arr, proj_t *proj, hashmap_t *projects)
{
	array_t *depends = &proj->props[PROJ_PROP_DEPENDS].arr;
	for (int i = 0; i < depends->count; i++) {
		prop_str_t *dname = array_get(depends, i);

		proj_t *dproj = NULL;
		if (hashmap_get(projects, dname->data, dname->len, (void **)&dproj)) {
			ERR("project doesn't exists: '%.*s'", dname->len, dname->data);
			continue;
		}

		if (array_index_cb(arr, &dname, proj_cmp_name) == -1) {
			array_add(arr, &dproj);
		}

		get_all_depends(arr, dproj, projects);
	}
}

static void calculate_depends(void *key, size_t ksize, void *value, void *priv)
{
	sln_t *sln   = priv;
	proj_t *proj = value;

	if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE) {
		array_t *depends = &proj->props[PROJ_PROP_DEPENDS].arr;
		array_init(&proj->all_depends, depends->capacity * 2, sizeof(proj_t *));
		get_all_depends(&proj->all_depends, proj, &sln->projects);
	}
}

static void calculate_includes(void *key, size_t ksize, void *value, void *priv)
{
	sln_t *sln   = priv;
	proj_t *proj = value;

	array_t *includes = &proj->props[PROJ_PROP_INCLUDES].arr;
	array_init(&proj->includes, includes->capacity, sizeof(proj_t *));
	for (int i = 0; i < includes->count; i++) {
		prop_str_t *iname = array_get(includes, i);

		proj_t *iproj = NULL;
		if (hashmap_get(&sln->projects, iname->data, iname->len, (void **)&iproj)) {
			ERR("project doesn't exists: '%.*s'", iname->len, iname->data);
			continue;
		}

		array_add(&proj->includes, &iproj);
	}
}

int sln_read(sln_t *sln, const path_t *path)
{
	sln->path      = *path;
	sln->file_path = *path;

	if (path_child(&sln->file_path, "Solution.txt", 12)) {
		return 1;
	}

	if ((sln->data.len = (unsigned int)file_read(sln->file_path.path, 1, sln->file, DATA_LEN)) == -1) {
		return 1;
	}

	sln->data.path = sln->file_path.path;
	sln->data.data = sln->file;
	sln->data.cur  = 0;

	int ret = props_parse_file(&sln->data, sln->props, s_sln_props, sizeof(s_sln_props));

	path_t name = { 0 };
	path_init(&name, sln->props[SLN_PROP_NAME].value.data, sln->props[SLN_PROP_NAME].value.len);
	path_child_s(&name, "sln", 3, '.');

	unsigned char buf[256] = { 0 };
	md5(name.path, name.len, buf, sizeof(buf), sln->guid, sizeof(sln->guid));

	array_t *dirs		    = &sln->props[SLN_PROP_DIRS].arr;
	path_t child_path	    = sln->path;
	unsigned int child_path_len = child_path.len;

	hashmap_create(&sln->projects, 16);
	hashmap_create(&sln->dirs, 16);

	read_dir_data_t read_dir_data = {
		.sln	= sln,
		.parent = NULL,
	};

	for (int i = 0; i < dirs->count; i++) {
		prop_str_t *dir = array_get(dirs, i);
		path_child(&child_path, dir->data, dir->len);
		if (folder_exists(child_path.path)) {
			if (read_dir(&child_path, dir->data, &read_dir_data)) {
				ret += 1;
				continue;
			}
		} else {
			ERR_LOGICS("Folder '%.*s' doesn't exists", dir->path, dir->line + 1, dir->start - dir->line_start + 1, dir->len, dir->data);
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
	     sln->path.len, sln->path.path, sln->file_path.len, sln->file_path.path, sln->guid);

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
