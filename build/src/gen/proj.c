#include "proj.h"

#include "dir.h"
#include "sln.h"

#include "common.h"

#include "md5.h"

static const prop_pol_t s_proj_props[] = {
	[PROJ_PROP_NAME]     = { .name = STR("NAME"), },
	[PROJ_PROP_TYPE]     = { .name = STR("TYPE"), .str_table = s_proj_types, .str_table_len = __PROJ_TYPE_MAX },
	[PROJ_PROP_LANGS]    = { .name = STR("LANGS"), .str_table = s_langs, .str_table_len = __LANG_MAX, .arr = 1 },
	[PROJ_PROP_SOURCE]   = { .name = STR("SOURCE"), },
	[PROJ_PROP_INCLUDE]  = { .name = STR("INCLUDE"), },
	[PROJ_PROP_ENCLUDE]  = { .name = STR("ENCLUDE"), },
	[PROJ_PROP_DEPENDS]  = { .name = STR("DEPENDS"), .arr = 1 },
	[PROJ_PROP_DEPEND]   = { .name = STR("DEPEND"),  .arr = 1 },
	[PROJ_PROP_INCLUDES] = { .name = STR("INCLUDES"), .arr = 1 },
	[PROJ_PROP_DEFINES]  = { .name = STR("DEFINES"), .arr = 1 },
	[PROJ_PROP_LIBDIRS]  = { .name = STR("LIBDIRS"), .arr = 1 },
	[PROJ_PROP_WDIR]     = { .name = STR("WDIR"),  },
	[PROJ_PROP_CHARSET]  = { .name = STR("CHARSET"), .str_table = s_charsets, .str_table_len = __CHARSET_MAX },
	[PROJ_PROP_OUTDIR]   = { .name = STR("OUTDIR"), },
	[PROJ_PROP_INTDIR]   = { .name = STR("INTDIR"), },
	[PROJ_PROP_CFLAGS]   = { .name = STR("CFLAGS"), .str_table = s_cflags, .str_table_len = __CFLAG_MAX, .arr = 1 },
	[PROJ_PROP_LDFLAGS]  = { .name = STR("LDFLAGS"), .str_table = s_ldflags, .str_table_len = __LDFLAG_MAX, .arr = 1 },
	[PROJ_PROP_LINK]     = { .name = STR("LINK"),.arr = 1},
	[PROJ_PROP_ARGS]     = { .name = STR("ARGS"), },
};

static void replace_prop(prop_t *proj_prop, const prop_t *sln_prop)
{
	if (!(proj_prop->flags & PROP_SET)) {
		prop_free(proj_prop);
		*proj_prop = *sln_prop;
		proj_prop->flags &= ~PROP_ARR;
	}
}

int proj_read(proj_t *proj, const path_t *sln_path, const path_t *path, const struct dir_s *parent, const prop_t *sln_props)
{
	proj->file_path = *path;
	pathv_path(&proj->path, &proj->file_path);
	pathv_sub(&proj->rel_path, &proj->file_path, sln_path);
	pathv_folder(&proj->folder, &proj->file_path);

	proj->dir = (pathv_t){
		.len  = (proj->rel_path.len < (proj->folder.len + 1)) ? 0 : (proj->rel_path.len - proj->folder.len - 1),
		.path = proj->rel_path.path,
	};

	proj->parent = parent;

	if (path_child(&proj->file_path, "Project.txt", 11)) {
		return 1;
	}

	if ((proj->data.len = (unsigned int)file_read_t(proj->file_path.path, proj->file, DATA_LEN)) == -1) {
		return 1;
	}

	proj->data.path = proj->file_path.path;
	proj->data.data = proj->file;

	int ret = props_parse_file(proj->data, proj->props, s_proj_props, sizeof(s_proj_props));

	if (!proj->props[PROJ_PROP_NAME].flags & PROP_SET) {
		ERR("%.*s: project name is not set", proj->file_path.len, proj->file_path.path);
		ret++;
		return ret;
	}

	proj->name = &proj->props[PROJ_PROP_NAME].value;

	path_t rel_path_name = { 0 };
	path_init(&rel_path_name, proj->rel_path.path, proj->rel_path.len);
	path_child(&rel_path_name, proj->name->data, proj->name->len);
	path_child_s(&rel_path_name, "vcxproj", 7, '.');
	unsigned char buf[256] = { 0 };
	md5(rel_path_name.path, rel_path_name.len, buf, sizeof(buf), proj->guid, sizeof(proj->guid));

	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		prop_str_t *source = &proj->props[PROJ_PROP_SOURCE].value;
		path_t path	   = { 0 };
		path_init(&path, proj->path.path, proj->path.len);
		if (path_child(&path, source->data, source->len)) {
			ret++;
		} else {
			if (!folder_exists(path.path)) {
				ERR_LOGICS("source folder does not exists '%.*s'", source->path, source->line, source->col, source->len, source->data);
				ret++;
			}
		}
	}

	if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
		prop_str_t *include = &proj->props[PROJ_PROP_INCLUDE].value;
		path_t path	    = { 0 };
		path_init(&path, proj->path.path, proj->path.len);
		if (path_child(&path, include->data, include->len)) {
			ret++;
		} else {
			if (!folder_exists(path.path)) {
				ERR_LOGICS("include folder does not exists '%.*s'", include->path, include->line, include->col, include->len, include->data);
				ret++;
			}
		}
	}

	replace_prop(&proj->props[PROJ_PROP_LANGS], &sln_props[SLN_PROP_LANGS]);
	replace_prop(&proj->props[PROJ_PROP_CHARSET], &sln_props[SLN_PROP_CHARSET]);
	replace_prop(&proj->props[PROJ_PROP_CFLAGS], &sln_props[SLN_PROP_CFLAGS]);
	replace_prop(&proj->props[PROJ_PROP_OUTDIR], &sln_props[SLN_PROP_OUTDIR]);
	replace_prop(&proj->props[PROJ_PROP_INTDIR], &sln_props[SLN_PROP_INTDIR]);

	return ret;
}

void proj_print(proj_t *proj)
{
	INFP("Project\n"
	     "    Path   : %.*s\n"
	     "    File   : %.*s\n"
	     "    Rel    : %.*s\n"
	     "    Dir    : %.*s\n"
	     "    Folder : %.*s\n"
	     "    Name   : %.*s\n"
	     "    GUID   : %s",
	     proj->path.len, proj->path.path, proj->file_path.len, proj->file_path.path, proj->rel_path.len, proj->rel_path.path, proj->dir.len, proj->dir.path,
	     proj->folder.len, proj->folder.path, proj->name->len, proj->name->data, proj->guid);

	if (proj->parent) {
		INFP("    Parent : %.*s", (unsigned int)proj->parent->folder.len, proj->parent->folder.path);
	} else {
		INFP("%s", "    Parent :");
	}

	props_print(proj->props, s_proj_props, sizeof(s_proj_props));

	INFP("%s", "    Depends:");
	for (int i = 0; i < proj->all_depends.count; i++) {
		const proj_t *dproj = *(proj_t **)array_get(&proj->all_depends, i);
		INFP("        '%.*s'", dproj->name->len, dproj->name->data);
	}

	INFP("%s", "    Includes:");
	for (int i = 0; i < proj->includes.count; i++) {
		const proj_t *dproj = *(proj_t **)array_get(&proj->includes, i);
		INFP("        '%.*s'", dproj->name->len, dproj->name->data);
	}

	INFF();
}

void proj_free(proj_t *proj)
{
	props_free(proj->props, s_proj_props, sizeof(s_proj_props));
	array_free(&proj->all_depends);
	array_free(&proj->includes);
}
