#include "gen/proj.h"

#include "dir.h"
#include "gen/sln.h"

#include "common.h"
#include "defines.h"

#include "md5.h"

static const prop_pol_t s_proj_props[] = {
	[PROJ_PROP_NAME]     = { .name = STRS("NAME") },
	[PROJ_PROP_TYPE]     = { .name = STRS("TYPE"), .str_table = s_proj_types, .str_table_len = __PROJ_TYPE_MAX },
	[PROJ_PROP_LANGS]    = { .name = STRS("LANGS"), .str_table = s_langs, .str_table_len = __LANG_MAX, .arr = 1 },
	[PROJ_PROP_SOURCE]   = { .name = STRS("SOURCE"), .arr = 1 },
	[PROJ_PROP_URL]	     = { .name = STRS("URL") },
	[PROJ_PROP_FORMAT]   = { .name = STRS("FORMAT") },
	[PROJ_PROP_INCLUDE]  = { .name = STRS("INCLUDE"), .arr = 1 },
	[PROJ_PROP_ENCLUDE]  = { .name = STRS("ENCLUDE") },
	[PROJ_PROP_DEPENDS]  = { .name = STRS("DEPENDS"), .arr = 1 },
	[PROJ_PROP_DEPEND]   = { .name = STRS("DEPEND"), .arr = 1 },
	[PROJ_PROP_INCLUDES] = { .name = STRS("INCLUDES"), .arr = 1 },
	[PROJ_PROP_DEFINES]  = { .name = STRS("DEFINES"), .arr = 1 },
	[PROJ_PROP_LIBDIRS]  = { .name = STRS("LIBDIRS"), .arr = 1 },
	[PROJ_PROP_WDIR]     = { .name = STRS("WDIR") },
	[PROJ_PROP_CHARSET]  = { .name = STRS("CHARSET"), .str_table = s_charsets, .str_table_len = __CHARSET_MAX },
	[PROJ_PROP_OUTDIR]   = { .name = STRS("OUTDIR") },
	[PROJ_PROP_INTDIR]   = { .name = STRS("INTDIR") },
	[PROJ_PROP_CFLAGS]   = { .name = STRS("CFLAGS"), .str_table = s_cflags, .str_table_len = __CFLAG_MAX, .arr = 1 },
	[PROJ_PROP_CCFLAGS]  = { .name = STRS("CCFLAGS"), .arr = 1 },
	[PROJ_PROP_LDFLAGS]  = { .name = STRS("LDFLAGS"), .str_table = s_ldflags, .str_table_len = __LDFLAG_MAX, .arr = 1 },
	[PROJ_PROP_LINK]     = { .name = STRS("LINK"), .arr = 1 },
	[PROJ_PROP_EXPORT]   = { .name = STRS("EXPORT"), .arr = 1 },
	[PROJ_PROP_REQUIRE]  = { .name = STRS("REQUIRE"), .arr = 1 },
	[PROJ_PROP_CONFIG]   = { .name = STRS("CONFIG") },
	[PROJ_PROP_TARGET]   = { .name = STRS("TARGET") },
	[PROJ_PROP_RUN]	     = { .name = STRS("RUN") },
	[PROJ_PROP_DRUN]     = { .name = STRS("DRUN") },
	[PROJ_PROP_PROGRAM]  = { .name = STRS("PROGRAM") },
	[PROJ_PROP_FILES]    = { .name = STRS("FILES"), .arr = 1 },
	[PROJ_PROP_SIZE]     = { .name = STRS("SIZE") },
	[PROJ_PROP_FILENAME] = { .name = STRS("FILENAME") },
	[PROJ_PROP_ARGS]     = { .name = STRS("ARGS") },
	[PROJ_PROP_ARTIFACT] = { .name = STRS("ARTIFACT") },
};

static void replace_prop(prop_t *proj_prop, const prop_t *sln_prop)
{
	if (!(proj_prop->flags & PROP_SET)) {
		*proj_prop     = *sln_prop;
		proj_prop->ref = 1;
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

	if (path_child(&proj->file_path, CSTR("Project.txt")) == NULL) {
		return 1;
	}

	if ((proj->data.val.len = file_read_t(proj->file_path.path, proj->file, DATA_LEN)) == -1) {
		return 1;
	}

	proj->data.path = proj->file_path.path;
	proj->data.val	= strb(proj->file, sizeof(proj->file), proj->data.val.len);

	int ret = props_parse_file(proj->data, proj->props, s_proj_props, sizeof(s_proj_props));

	if (!proj->props[PROJ_PROP_NAME].flags & PROP_SET) {
		ERR("%.*s: project name is not set", (int)proj->file_path.len, proj->file_path.path);
		ret++;
		return ret;
	}

	proj->name = &proj->props[PROJ_PROP_NAME].value;

	path_t rel_path_name = { 0 };
	path_init(&rel_path_name, proj->rel_path.path, proj->rel_path.len);
	path_child(&rel_path_name, proj->name->val.data, proj->name->val.len);
	path_child_s(&rel_path_name, CSTR("vcxproj"), '.');
	byte buf[256] = { 0 };
#if defined(C_LINUX)
	convert_backslash(rel_path_name.path, rel_path_name.len, rel_path_name.path, rel_path_name.len);
#endif
	md5(rel_path_name.path, rel_path_name.len, buf, sizeof(buf), proj->guid, sizeof(proj->guid));

	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;

		for (uint i = 0; i < sources->cnt; i++) {
			prop_str_t *source = arr_get(sources, i);

			path_t path = { 0 };
			path_init(&path, proj->path.path, proj->path.len);
			if (path_child(&path, source->val.data, source->val.len) == NULL) {
				ret++;
			} else {
				if (!folder_exists(path.path)) {
					ERR_LOGICS("source folder does not exists '%.*s'", source->path, source->line, source->col, (int)source->val.len,
						   source->val.data);
					ret++;
				}
			}
		}
	}

	if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
		const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;

		for (uint i = 0; i < includes->cnt; i++) {
			prop_str_t *include = arr_get(includes, i);

			path_t path = { 0 };
			path_init(&path, proj->path.path, proj->path.len);
			if (path_child(&path, include->val.data, include->val.len) == NULL) {
				ret++;
			} else {
				if (!folder_exists(path.path)) {
					ERR_LOGICS("include folder does not exists '%.*s'", include->path, include->line, include->col, (int)include->val.len,
						   include->val.data);
					ret++;
				}
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
	     (int)proj->path.len, proj->path.path, (int)proj->file_path.len, proj->file_path.path, (int)proj->rel_path.len, proj->rel_path.path, (int)proj->dir.len,
	     proj->dir.path, (int)proj->folder.len, proj->folder.path, (int)proj->name->val.len, proj->name->val.data, proj->guid);

	if (proj->parent) {
		INFP("    Parent : %.*s", (int)proj->parent->folder.len, proj->parent->folder.path);
	} else {
		INFP("%s", "    Parent :");
	}

	props_print(proj->props, s_proj_props, sizeof(s_proj_props));

	INFP("%s", "    Depends:");
	for (uint i = 0; i < proj->all_depends.cnt; i++) {
		const proj_t *dproj = *(proj_t **)arr_get(&proj->all_depends, i);
		INFP("        '%.*s'", (int)dproj->name->val.len, dproj->name->val.data);
	}

	INFP("%s", "    Includes:");
	for (uint i = 0; i < proj->includes.cnt; i++) {
		const proj_t *dproj = *(proj_t **)arr_get(&proj->includes, i);
		INFP("        '%.*s'", (int)dproj->name->val.len, dproj->name->val.data);
	}

	INFF();
}

int proj_runnable(const proj_t *proj)
{
	return proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE || proj->props[PROJ_PROP_RUN].flags & PROP_SET;
}

int proj_coverable(const proj_t *proj)
{
	const prop_t *langs = &proj->props[PROJ_PROP_LANGS];
	return (langs->flags & PROP_SET) && (langs->mask & ((1 << LANG_C) | (1 << LANG_CPP)));
}

void proj_free(proj_t *proj)
{
	props_free(proj->props, s_proj_props, sizeof(s_proj_props));
	arr_free(&proj->all_depends);
	arr_free(&proj->includes);
}
