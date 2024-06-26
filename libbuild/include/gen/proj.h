#ifndef PROJ_H
#define PROJ_H

#include "build.h"
#include "prop.h"

#include "arr.h"
#include "cstr.h"
#include "dict.h"
#include "make.h"
#include "path.h"
#include "str.h"

typedef enum proj_prop_e {
	PROJ_PROP_NAME,
	PROJ_PROP_TYPE,
	PROJ_PROP_LANGS,
	PROJ_PROP_SOURCE,
	PROJ_PROP_URL,
	PROJ_PROP_FORMAT,
	PROJ_PROP_INCLUDE,
	PROJ_PROP_ENCLUDE,
	PROJ_PROP_DEPENDS,
	PROJ_PROP_DDEPENDS,
	PROJ_PROP_DEPEND,
	PROJ_PROP_INCLUDES,
	PROJ_PROP_DEFINES,
	PROJ_PROP_LIBDIRS,
	PROJ_PROP_WDIR,
	PROJ_PROP_CHARSET,
	PROJ_PROP_OUTDIR,
	PROJ_PROP_INTDIR,
	PROJ_PROP_CFLAGS,
	PROJ_PROP_CCFLAGS,
	PROJ_PROP_LDFLAGS,
	PROJ_PROP_LINK,
	PROJ_PROP_EXPORT,
	PROJ_PROP_REQUIRE,
	PROJ_PROP_CONFIG,
	PROJ_PROP_TARGET,
	PROJ_PROP_RUN,
	PROJ_PROP_DRUN,
	PROJ_PROP_PROGRAM,
	PROJ_PROP_FILES,
	PROJ_PROP_SIZE,
	PROJ_PROP_FILENAME,
	PROJ_PROP_ARGS,
	PROJ_PROP_ARTIFACT,

	__PROJ_PROP_MAX,
} proj_prop_t;

typedef enum proj_type_e {
	PROJ_TYPE_UNKNOWN,
	PROJ_TYPE_LIB,
	PROJ_TYPE_EXE,
	PROJ_TYPE_EXT,
	PROJ_TYPE_BIN,
	PROJ_TYPE_ELF,
	PROJ_TYPE_FAT12,

	__PROJ_TYPE_MAX,
} proj_type_t;

// clang-format off
static const str_t s_proj_types[] = {
	[PROJ_TYPE_UNKNOWN] = STRS(""),
	[PROJ_TYPE_LIB]	    = STRS("LIB"),
	[PROJ_TYPE_EXE]	    = STRS("EXE"),
	[PROJ_TYPE_EXT]	    = STRS("EXT"),
	[PROJ_TYPE_BIN]	    = STRS("BIN"),
	[PROJ_TYPE_ELF]	    = STRS("ELF"),
	[PROJ_TYPE_FAT12]   = STRS("FAT12"),
};
// clang-format on

typedef enum lang_e {
	LANG_NONE,
	LANG_ASM,
	LANG_C,
	LANG_CPP,

	__LANG_MAX,
} lang_t;

// clang-format off
static const str_t s_langs[] = {
	[LANG_NONE]    = STRS("NONE"),
	[LANG_ASM]     = STRS("ASM"),
	[LANG_C]       = STRS("C"),
	[LANG_CPP]     = STRS("CPP"),
};
// clang-format on

typedef enum charset_e {
	CHARSET_UNKNOWN,
	CHARSET_UNICODE,
	CHARSET_MULTI_BYTE,

	__CHARSET_MAX,
} charset_t;

static const str_t s_charsets[] = {
	[CHARSET_UNKNOWN]    = STRS(""),
	[CHARSET_UNICODE]    = STRS("Unicode"),
	[CHARSET_MULTI_BYTE] = STRS("MultiByte"),
};

typedef enum cflag_e {
	CFLAG_NONE,
	CFLAG_STD_C99,
	CFLAG_FREESTANDING,

	__CFLAG_MAX,
} cflag_t;

static const str_t s_cflags[] = {
	[CFLAG_NONE]	     = STRS("NONE"),
	[CFLAG_STD_C99]	     = STRS("STD_C99"),
	[CFLAG_FREESTANDING] = STRS("FREESTANDING"),
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

typedef enum link_type_e {
	LINK_TYPE_STATIC,
	LINK_TYPE_DYNAMIC,
} link_type_t;

static const str_t s_ldflags[] = {
	[LDFLAG_NONE]			 = STRS("NONE"),
	[LDFLAG_WHOLEARCHIVE]		 = STRS("WHOLEARCHIVE"),
	[LDFLAG_ALLOWMULTIPLEDEFINITION] = STRS("ALLOWMULTIPLEDEFINITION"),
	[LDFLAG_MATH]			 = STRS("MATH"),
	[LDFLAG_X11]			 = STRS("X11"),
	[LDFLAG_GL]			 = STRS("GL"),
	[LDFLAG_GLX]			 = STRS("GLX"),
};

typedef struct proj_s {
	path_t path;
	pathv_t dir;
	pathv_t rel_dir;
	str_t name;
	path_t gen_path;
	path_t gen_path_d;
	char file[1024];
	prop_str_t data;
	prop_t props[__PROJ_PROP_MAX];
	char guid[37];
	char guid2[37];
	arr_t all_depends;
	arr_t includes;
	const struct dir_s *parent;
	make_t make;
} proj_t;

typedef struct proj_dep_s {
	const proj_t *proj;
	link_type_t link_type;
} proj_dep_t;

int proj_read(build_t *build, proj_t *proj, const pathv_t *sln_path, const path_t *path, const struct dir_s *parent, const prop_t *sln_props);
void proj_print(proj_t *proj);

int proj_runnable(const proj_t *proj);
int proj_coverable(const proj_t *proj);

void proj_free(proj_t *proj);

#endif
