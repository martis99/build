#include "gen/sln.h"

#include "dir.h"
#include "gen/proj.h"

#include "build.h"
#include "common.h"

#include "md5.h"

static const prop_pol_t s_sln_props[] = {
	[SLN_PROP_NAME]	   = { .name = STRS("NAME"), },
	[SLN_PROP_LANGS]   = { .name = STRS("LANGS"),  .str_table = s_langs, .str_table_len = __LANG_MAX, .flags = PPF_ARR },
	[SLN_PROP_DIRS]	   = { .name = STRS("DIRS"),  .flags = PPF_ARR | PPF_DIR },
	[SLN_PROP_STARTUP] = { .name = STRS("STARTUP"), },
	[SLN_PROP_CONFIGS] = { .name = STRS("CONFIGS"), .flags = PPF_ARR},
	[SLN_PROP_ARCHS]   = { .name = STRS("ARCHS"), .flags = PPF_ARR },
	[SLN_PROP_CHARSET] = { .name = STRS("CHARSET"), .str_table = s_charsets, .str_table_len = __CHARSET_MAX },
	[SLN_PROP_FLAGS]  = { .name = STRS("CFLAGS"), .str_table = s_flags, .str_table_len = __FLAG_MAX, .flags = PPF_ARR },
	[SLN_PROP_OUTDIR]  = { .name = STRS("OUTDIR"), .def = STRS("$(SLN_DIR)bin\\$(CONFIG)-$(ARCH)\\$(PROJ_DIR)")},
	[SLN_PROP_INTDIR]  = { .name = STRS("INTDIR"), .def = STRS("$(SLN_DIR)bin\\$(CONFIG)-$(ARCH)\\$(PROJ_DIR)int\\")},
};

typedef struct read_dir_data_s {
	build_t *build;
	sln_t *sln;
	dir_t *parent;
} read_dir_data_t;

static int read_dir(path_t *path, const char *folder, void *priv)
{
	int ret = 0;

	MSG("entering directory: %s", path->path);

	path_t file_path = *path;
	if (path_child(&file_path, CSTR("Project.txt")) == NULL) {
		return 1;
	}

	read_dir_data_t *data = priv;
	if (file_exists(file_path.path)) {
		proj_t *proj = mem_calloc(1, sizeof(proj_t));
		ret += proj_read(data->build, proj, &data->sln->dir, &file_path, data->parent, data->sln->props);
		const str_t *name = &proj->name;
		if (dict_get(&data->sln->projects, name->data, name->len, NULL)) {
			dict_set(&data->sln->projects, name->data, name->len, proj);
		} else {
			proj_free(proj);
			mem_free(proj, sizeof(proj_t));
			return -1;
		}

		return ret;
	}

	path_set_len(&file_path, path->len);
	if (path_child(&file_path, CSTR("Directory.txt")) == NULL) {
		return 1;
	}

	dir_t *dir = mem_calloc(1, sizeof(dir_t));
	if (dict_get(&data->sln->dirs, path->path, path->len, NULL)) {
		dict_set(&data->sln->dirs, path->path, path->len, dir);
		ret += dir_read(data->build, dir, &data->sln->dir, &file_path, read_dir, data->parent, data);
	} else {
		//ERR_LOGICS("Direcotry '%.*s' with the same path already exists", name->path, name->line + 1, name->start - name->line_start + 1, name->len, name->data);
		mem_free(dir, sizeof(dir_t));
	}

	return ret;
}

static int proj_eq_name(const void *proj, const void *name)
{
	return str_eq((*(const proj_t **)proj)->name, *(const str_t *)name);
}

static void get_all_depends(arr_t *arr, const proj_t *proj, dict_t *projects)
{
	const arr_t *depends = &proj->props[PROJ_PROP_DEPENDS].arr;
	for (uint i = 0; i < depends->cnt; i++) {
		const prop_str_t *dname = arr_get(depends, i);

		const proj_t *dproj = NULL;
		if (dict_get(projects, dname->val.data, dname->val.len, (void **)&dproj)) {
			ERR("project doesn't exists: '%.*s'", (int)dname->val.len, dname->val.data);
			continue;
		}

		get_all_depends(arr, dproj, projects);

		if (arr_index_cmp(arr, &dname->val, proj_eq_name) == -1) {
			proj_dep_t dep = {
				.proj	   = dproj,
				.link_type = LINK_TYPE_STATIC,
			};
			arr_app(arr, &dep);
		}
	}

	const arr_t *ddepends = &proj->props[PROJ_PROP_DDEPENDS].arr;
	for (uint i = 0; i < ddepends->cnt; i++) {
		const prop_str_t *dname = arr_get(ddepends, i);

		const proj_t *dproj = NULL;
		if (dict_get(projects, dname->val.data, dname->val.len, (void **)&dproj)) {
			ERR("project doesn't exists: '%.*s'", (int)dname->val.len, dname->val.data);
			continue;
		}

		if (arr_index_cmp(arr, &dname->val, proj_eq_name) == -1) {
			proj_dep_t dep = {
				.proj	   = dproj,
				.link_type = LINK_TYPE_DYNAMIC,
			};
			arr_app(arr, &dep);
		}
	}
}

static void calculate_depends(proj_t *proj, sln_t *sln)
{
	arr_t *depends = &proj->props[PROJ_PROP_DEPENDS].arr;
	if (depends->cap == 0) {
		return;
	}

	arr_init(&proj->all_depends, depends->cap * 2, sizeof(proj_dep_t));
	get_all_depends(&proj->all_depends, proj, &sln->projects);

	for (uint i = 0; i < proj->all_depends.cnt; i++) {
		const proj_dep_t *dep = arr_get(&proj->all_depends, i);

		const arr_t *depend = &dep->proj->props[PROJ_PROP_DEPEND].arr;
		for (uint i = 0; i < depend->cnt; i++) {
			const prop_str_t *dpname = arr_get(depend, i);

			const proj_t *dpproj = NULL;
			if (dict_get(&sln->projects, dpname->val.data, dpname->val.len, (void **)&dpproj)) {
				ERR("project doesn't exists: '%.*s'", (int)dpname->val.len, dpname->val.data);
				continue;
			}

			if (arr_index_cmp(&proj->all_depends, &dpname->val, proj_eq_name) == -1) {
				proj_dep_t dep = {
					.proj	   = dpproj,
					.link_type = LINK_TYPE_STATIC,
				};
				arr_app(&proj->all_depends, &dep);
			}
		}
	}
}

static void get_all_includes(arr_t *arr, const proj_t *proj, dict_t *projects)
{
	const arr_t *includes = &proj->props[PROJ_PROP_INCLUDES].arr;
	for (uint i = 0; i < includes->cnt; i++) {
		const prop_str_t *iname = arr_get(includes, i);

		const proj_t *iproj = NULL;
		if (dict_get(projects, iname->val.data, iname->val.len, (void **)&iproj)) {
			ERR("project doesn't exists: '%.*s'", (int)iname->val.len, iname->val.data);
			continue;
		}

		get_all_includes(arr, iproj, projects);

		if (arr_index_cmp(arr, &iname->val, proj_eq_name) == -1) {
			arr_app(arr, &iproj);
		}
	}
}

static void calculate_includes(proj_t *proj, sln_t *sln)
{
	if (!(proj->props[PROJ_PROP_INCLUDES].flags & PROP_SET)) {
		return;
	}

	arr_t *includes = &proj->props[PROJ_PROP_INCLUDES].arr;
	arr_init(&proj->includes, includes->cap * 2, sizeof(proj_t *));
	get_all_includes(&proj->includes, proj, &sln->projects);
}

static void calculate_build_older(arr_t *arr, const proj_t *proj, dict_t *projects)
{
	const arr_t *depends = &proj->props[PROJ_PROP_DEPENDS].arr;
	for (uint i = 0; i < depends->cnt; i++) {
		const prop_str_t *dname = arr_get(depends, i);

		const proj_t *dproj = NULL;
		if (dict_get(projects, dname->val.data, dname->val.len, (void **)&dproj)) {
			ERR("project doesn't exists: '%.*s'", (int)dname->val.len, dname->val.data);
			continue;
		}

		calculate_build_older(arr, dproj, projects);

		if (arr_index_cmp(arr, &dname->val, proj_eq_name) == -1) {
			arr_app(arr, &dproj);
		}
	}

	const arr_t *ddepends = &proj->props[PROJ_PROP_DDEPENDS].arr;
	for (uint i = 0; i < ddepends->cnt; i++) {
		const prop_str_t *dname = arr_get(ddepends, i);

		const proj_t *dproj = NULL;
		if (dict_get(projects, dname->val.data, dname->val.len, (void **)&dproj)) {
			ERR("project doesn't exists: '%.*s'", (int)dname->val.len, dname->val.data);
			continue;
		}

		calculate_build_older(arr, dproj, projects);

		if (arr_index_cmp(arr, &dname->val, proj_eq_name) == -1) {
			arr_app(arr, &dproj);
		}
	}

	if (arr_index_cmp(arr, &proj->name, proj_eq_name) == -1) {
		arr_app(arr, &proj);
	}
}

int sln_read(sln_t *sln, const path_t *path)
{
	sln->path = *path;
	sln->dir  = pathv_path(path);

	if (path_child(&sln->path, CSTR("Solution.txt")) == NULL) {
		return 1;
	}

	if (!file_exists(sln->path.path)) {
		ERR_INPUT("file doesn't exists: '%.*s'", __func__, (int)sln->path.len, sln->path.path);
		return 1;
	}

	if ((sln->data.val.len = file_read_t(sln->path.path, sln->file, DATA_LEN)) == -1) {
		return 1;
	}

	sln->data.path = sln->path.path;
	sln->data.val  = strb(sln->file, sizeof(sln->file), sln->data.val.len);

	build_t build = { 0 };

	ini_prs_init(&build.ini_prs);

	int ret = props_parse_file(sln->data, &build.ini_prs, sln->props, s_sln_props, sizeof(s_sln_props));

	path_t name = { 0 };
	path_init(&name, sln->props[SLN_PROP_NAME].value.val.data, sln->props[SLN_PROP_NAME].value.val.len);
	path_child_s(&name, CSTR("sln"), '.');

	byte buf[256] = { 0 };
	md5(name.path, name.len, buf, sizeof(buf), sln->guid, sizeof(sln->guid));

	arr_t *dirs	  = &sln->props[SLN_PROP_DIRS].arr;
	path_t child_path = { 0 };
	path_init(&child_path, sln->dir.path, sln->dir.len);
	size_t child_path_len = child_path.len;

	dict_init(&sln->projects, 16);
	dict_init(&sln->dirs, 16);

	read_dir_data_t read_dir_data = {
		.build	= &build,
		.sln	= sln,
		.parent = NULL,
	};

	prop_def(sln->props, s_sln_props, sizeof(s_sln_props));

	for (uint i = 0; i < dirs->cnt; i++) {
		prop_str_t *dir = arr_get(dirs, i);
		path_child_dir(&child_path, dir->val.data, dir->val.len);
		if (folder_exists(child_path.path)) {
			const int r = read_dir(&child_path, dir->val.data, &read_dir_data);
			if (r == -1) {
				dir->val.data = NULL;
			} else {
				ret += r;
			}
		} else {
			ERR_LOGICS("Folder '%.*s' doesn't exists", dir->path, dir->line, dir->col, (int)child_path.len, child_path.path);
			ret += 1;
		}
		child_path.len = child_path_len;
	}

	dict_foreach(&sln->projects, pair)
	{
		calculate_depends(pair->value, sln);
	}

	dict_foreach(&sln->projects, pair)
	{
		calculate_includes(pair->value, sln);
	}

	arr_init(&sln->build_order, sln->projects.count, sizeof(proj_t *));

	dict_foreach(&sln->projects, pair)
	{
		calculate_build_older(&sln->build_order, pair->value, &sln->projects);
	}

	ini_prs_free(&build.ini_prs);

	return ret;
}

void sln_print(sln_t *sln)
{
	INFP("Solution\n"
	     "    Path: %.*s\n"
	     "    Dir:  %.*s\n"
	     "    GUID: %s",
	     (int)sln->path.len, sln->path.path, (int)sln->dir.len, sln->dir.path, sln->guid);

	props_print(sln->props, s_sln_props, sizeof(s_sln_props));

	proj_t **pproj;
	INFP("%s", "Build order:");
	arr_foreach(&sln->build_order, pproj)
	{
		proj_t *proj = *pproj;
		INFP("    %.*s", (int)proj->name.len, proj->name.data);
	}

	dict_foreach(&sln->dirs, pair)
	{
		dir_print(pair->value);
	}

	dict_foreach(&sln->projects, pair)
	{
		proj_print(pair->value);
	}
}

static void free_dir(void *key, size_t ksize, void *value, void *priv)
{
	dir_free(value);
	mem_free(value, sizeof(dir_t));
}

void sln_free(sln_t *sln)
{
	arr_free(&sln->build_order);

	dict_foreach(&sln->projects, pair)
	{
		proj_free(pair->value);
		mem_free(pair->value, sizeof(proj_t));
	}

	dict_free(&sln->projects);

	dict_foreach(&sln->dirs, pair)
	{
		dir_free(pair->value);
		mem_free(pair->value, sizeof(dir_t));
	}
	dict_free(&sln->dirs);

	props_free(sln->props, s_sln_props, sizeof(s_sln_props));
}
