#ifndef PROJ_H
#define PROJ_H

#include "prop.h"

#include "array.h"
#include "hash_map.h"
#include "path.h"
#include "str.h"

typedef enum proj_prop_e {
	PROJ_PROP_NAME,
	PROJ_PROP_TYPE,
	PROJ_PROP_LANGS,
	PROJ_PROP_SOURCE,
	PROJ_PROP_INCLUDE,
	PROJ_PROP_ENCLUDE,
	PROJ_PROP_DEPENDS,
	PROJ_PROP_DEPEND,
	PROJ_PROP_INCLUDES,
	PROJ_PROP_DEFINES,
	PROJ_PROP_LIBDIRS,
	PROJ_PROP_WDIR,
	PROJ_PROP_CHARSET,
	PROJ_PROP_OUTDIR,
	PROJ_PROP_INTDIR,
	PROJ_PROP_CFLAGS,
	PROJ_PROP_LDFLAGS,
	PROJ_PROP_LINK,
	PROJ_PROP_ARGS,

	__PROJ_PROP_MAX,
} proj_prop_t;

typedef enum proj_type_e {
	PROJ_TYPE_UNKNOWN,
	PROJ_TYPE_LIB,
	PROJ_TYPE_EXE,
	PROJ_TYPE_EXT,

	__PROJ_TYPE_MAX,
} proj_type_t;

static const str_t s_proj_types[] = {
	[PROJ_TYPE_UNKNOWN] = { "", 0 },
	[PROJ_TYPE_LIB]	    = { "LIB", 3 },
	[PROJ_TYPE_EXE]	    = { "EXE", 3 },
	[PROJ_TYPE_EXT]	    = { "EXT", 3 },
};

typedef enum lang_e {
	LANG_NONE,
	LANG_C,
	LANG_ASM,
	LANG_CPP,

	__LANG_MAX,
} lang_t;

// clang-format off
static const str_t s_langs[] = {
	[LANG_NONE]    = { "NONE", 4 },
	[LANG_C]       = { "C", 1 },
	[LANG_ASM]     = { "ASM", 3 },
	[LANG_CPP]     = { "CPP", 3 },
};
// clang-format on

typedef enum charset_e {
	CHARSET_UNKNOWN,
	CHARSET_UNICODE,
	CHARSET_MULTI_BYTE,

	__CHARSET_MAX,
} charset_t;

static const str_t s_charsets[] = {
	[CHARSET_UNKNOWN]    = { "", 0 },
	[CHARSET_UNICODE]    = { "Unicode", 7 },
	[CHARSET_MULTI_BYTE] = { "MultiByte", 9 },
};

typedef enum cflag_e {
	CFLAG_NONE,
	CFLAG_STD_C99,

	__CFLAG_MAX,
} cflag_t;

static const str_t s_cflags[] = {
	[CFLAG_NONE]	= { "NONE", 4 },
	[CFLAG_STD_C99] = { "STD_C99", 7 },
};

typedef enum ldflag_e {
	LDFLAG_NONE,
	LDFLAG_WHOLEARCHIVE,
	LDFLAG_ALLOWMULTIPLEDEFINITION,
	LDFLAG_MATH,
	LDFLAG_X11,
	LDFLAG_GL,
	LDFLAG_GLX,

	__LDFLAG_MAX,
} ldflag_t;

static const str_t s_ldflags[] = {
	[LDFLAG_NONE]			 = { "NONE", 4 },
	[LDFLAG_WHOLEARCHIVE]		 = { "WHOLEARCHIVE", 12 },
	[LDFLAG_ALLOWMULTIPLEDEFINITION] = { "ALLOWMULTIPLEDEFINITION", 23 },
	[LDFLAG_MATH]			 = { "MATH", 4 },
	[LDFLAG_X11]			 = { "X11", 3 },
	[LDFLAG_GL]			 = { "GL", 2 },
	[LDFLAG_GLX]			 = { "GLX", 3 },
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
	const prop_str_t *name;
	array_t all_depends;
	array_t includes;
	const struct dir_s *parent;
} proj_t;

int proj_read(proj_t *proj, const path_t *sln_path, const path_t *path, const struct dir_s *parent, const prop_t *sln_props);
void proj_print(proj_t *proj);

void proj_free(proj_t *proj);

#endif
