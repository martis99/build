#ifndef PROJ_H
#define PROJ_H

#include "types.h"
#include "utils_types.h"
#include "array.h"
#include "hash_map.h"

typedef enum proj_prop_e {
	PROJ_PROP_NAME,
	PROJ_PROP_TYPE,
	PROJ_PROP_SOURCE,
	PROJ_PROP_INCLUDE,
	PROJ_PROP_DEPENDS,
	PROJ_PROP_INCLUDES,
	PROJ_PROP_DEFINES,
	PROJ_PROP_LIBDIRS,
	PROJ_PROP_WDIR,
	PROJ_PROP_CHARSET,
	PROJ_PROP_OUTDIR,
	PROJ_PROP_INTDIR,

	__PROJ_PROP_MAX,
} proj_prop_t;

typedef enum proj_type_e {
	PROJ_TYPE_UNKNOWN,
	PROJ_TYPE_LIB,
	PROJ_TYPE_EXE,
	PROJ_TYPE_EXT,

	__PROJ_TYPE_MAX
} proj_type_t;

static const str_t s_proj_types[] = {
	[PROJ_TYPE_UNKNOWN] = {"", 0},
	[PROJ_TYPE_LIB] = {"LIB", 3},
	[PROJ_TYPE_EXE] = {"EXE", 3},
	[PROJ_TYPE_EXT] = {"EXT", 3},
};

typedef enum charset_e {
	CHARSET_UNKNOWN,
	CHARSET_UNICODE,
	CHARSET_MULTI_BYTE,

	__CHARSET_MAX
} charset_t;

static const str_t s_charsets[] = {
	[CHARSET_UNKNOWN] = {"", 0},
	[CHARSET_UNICODE] = {"Unicode", 7},
	[CHARSET_MULTI_BYTE] = {"MultiByte", 9},
};

typedef struct proj_s {
	pathv_t path;
	path_t file_path;
	pathv_t rel_path;
	pathv_t dir;
	pathv_t folder;
	char file[1024];
	prop_str_t data;
	prop_t props[__PROJ_PROP_MAX];
	char guid[37];
	array_t all_depends;
	const struct dir_s *parent;
} proj_t;

int proj_read(proj_t *proj, const path_t *sln_path, const path_t *path, const struct dir_s *parent);
void proj_print(proj_t *proj);
int proj_gen_cmake(const proj_t *proj, const hashmap_t *projects, const path_t *path, int lang, charset_t charset);
int proj_gen_make(const proj_t *proj, const hashmap_t *projects, const path_t *path);
int proj_gen_vs(const proj_t *proj, const hashmap_t *projects, const path_t *path, const array_t *configs, const array_t *platforms, const prop_t *charset, const prop_t *outdir, const prop_t *intdir);
void proj_free(proj_t *proj);

#endif