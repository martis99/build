#include "proj.h"

#include "dir.h"
#include "prop.h"
#include "utils.h"

#include "defines.h"
#include "md5.h"
#include "mem.h"

#include <string.h>

static const prop_pol_t s_proj_props[] = {
	[PROJ_PROP_NAME]     = { .name = "NAME", .parse = prop_parse_word },
	[PROJ_PROP_TYPE]     = { .name = "TYPE", .parse = prop_parse_word, .str_table = s_proj_types, .str_table_len = __PROJ_TYPE_MAX },
	[PROJ_PROP_LANGS]    = { .name = "LANGS", .parse = prop_parse_word, .str_table = s_langs, .str_table_len = __LANG_MAX, .dim = PROP_DIM_ARRAY },
	[PROJ_PROP_SOURCE]   = { .name = "SOURCE", .parse = prop_parse_path },
	[PROJ_PROP_INCLUDE]  = { .name = "INCLUDE", .parse = prop_parse_path },
	[PROJ_PROP_ENCLUDE]  = { .name = "ENCLUDE", .parse = prop_parse_path },
	[PROJ_PROP_DEPENDS]  = { .name = "DEPENDS", .parse = prop_parse_word, .dim = PROP_DIM_ARRAY },
	[PROJ_PROP_INCLUDES] = { .name = "INCLUDES", .parse = prop_parse_word, .dim = PROP_DIM_ARRAY },
	[PROJ_PROP_DEFINES]  = { .name = "DEFINES", .parse = prop_parse_word, .dim = PROP_DIM_ARRAY },
	[PROJ_PROP_LIBDIRS]  = { .name = "LIBDIRS", .parse = prop_parse_path, .dim = PROP_DIM_ARRAY },
	[PROJ_PROP_WDIR]     = { .name = "WDIR", .parse = prop_parse_path },
	[PROJ_PROP_CHARSET]  = { .name = "CHARSET", .parse = prop_parse_word, .str_table = s_charsets, .str_table_len = __CHARSET_MAX },
	[PROJ_PROP_OUTDIR]   = { .name = "OUTDIR", .parse = prop_parse_path },
	[PROJ_PROP_INTDIR]   = { .name = "INTDIR", .parse = prop_parse_path },
	[PROJ_PROP_CFLAGS]   = { .name = "CFLAGS", .parse = prop_parse_word, .str_table = s_cflags, .str_table_len = __CFLAG_MAX, .dim = PROP_DIM_ARRAY },
	[PROJ_PROP_LDFLAGS]  = { .name = "LDFLAGS", .parse = prop_parse_word, .str_table = s_ldflags, .str_table_len = __LDFLAG_MAX, .dim = PROP_DIM_ARRAY },
	[PROJ_PROP_LINK]     = { .name = "LINK", .parse = prop_parse_word, .dim = PROP_DIM_ARRAY },
	[PROJ_PROP_ARGS]     = { .name = "ARGS", .parse = prop_parse_printable },
};

int proj_read(proj_t *proj, const path_t *sln_path, const path_t *path, const struct dir_s *parent)
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

	if ((proj->data.len = (unsigned int)file_read(proj->file_path.path, 1, proj->file, DATA_LEN)) == -1) {
		return 1;
	}

	proj->data.path = proj->file_path.path;
	proj->data.data = proj->file;
	proj->data.cur	= 0;

	int ret = props_parse_file(&proj->data, proj->props, s_proj_props, sizeof(s_proj_props));

	if (!proj->props[PROJ_PROP_NAME].set) {
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

	if (proj->props[PROJ_PROP_SOURCE].set) {
		prop_str_t *source = &proj->props[PROJ_PROP_SOURCE].value;
		path_t path	   = { 0 };
		path_init(&path, proj->path.path, proj->path.len);
		if (path_child(&path, source->data, source->len)) {
			ret++;
		} else {
			if (!folder_exists(path.path)) {
				ERR_LOGICS("source folder does not exists '%.*s'", source->path, source->line + 1, source->start - source->line_start + 1, source->len,
					   source->data);
				ret++;
			}
		}
	}

	if (proj->props[PROJ_PROP_INCLUDE].set) {
		prop_str_t *include = &proj->props[PROJ_PROP_INCLUDE].value;
		path_t path	    = { 0 };
		path_init(&path, proj->path.path, proj->path.len);
		if (path_child(&path, include->data, include->len)) {
			ret++;
		} else {
			if (!folder_exists(path.path)) {
				ERR_LOGICS("include folder does not exists '%.*s'", include->path, include->line + 1, include->start - include->line_start + 1,
					   include->len, include->data);
				ret++;
			}
		}
	}

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
