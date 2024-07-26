#ifndef PROJ_H
#define PROJ_H

#include "build.h"
#include "prop.h"

#include "gen/cm/cmake.h"
#include "gen/mk/make.h"
#include "gen/pgc.h"

#include "arr.h"
#include "cstr.h"
#include "dict.h"
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
	PROJ_PROP_ENCLUDE, //TODO: Remove
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
	PROJ_PROP_FLAGS,
	PROJ_PROP_LINK, //TODO: Remove
	PROJ_PROP_EXPORT,
	PROJ_PROP_REQUIRE,
	PROJ_PROP_LIB,
	PROJ_PROP_DLIB,
	PROJ_PROP_COPYFILES,
	PROJ_PROP_CONFIG,
	PROJ_PROP_TARGET,
	PROJ_PROP_RUN,
	PROJ_PROP_DRUN,
	PROJ_PROP_PROGRAM,
	PROJ_PROP_HEADER,
	PROJ_PROP_FILES,
	PROJ_PROP_SIZE,
	PROJ_PROP_FILENAME,
	PROJ_PROP_ARGS,
	PROJ_PROP_ARTIFACT,

	__PROJ_PROP_MAX,
} proj_prop_t;

typedef enum proj_var_e {
	PROJ_VAR_SLNDIR,
	PROJ_VAR_PROJDIR,
	PROJ_VAR_PROJNAME,
	PROJ_VAR_CONFIG,
	PROJ_VAR_ARCH,

	__PROJ_VAR_MAX
} proj_var_t;

// clang-format off
static const str_t s_proj_vars[] = {
	[PROJ_VAR_SLNDIR]   = STRS("$(SLNDIR)"),
	[PROJ_VAR_PROJDIR]  = STRS("$(PROJDIR)"),
	[PROJ_VAR_PROJNAME] = STRS("$(PROJNAME)"),
	[PROJ_VAR_CONFIG]   = STRS("$(CONFIG)"),
	[PROJ_VAR_ARCH]	    = STRS("$(ARCH)"),
};
// clang-format on

typedef enum proj_type_e {
	PROJ_TYPE_UNKNOWN,
	PROJ_TYPE_EXE,
	PROJ_TYPE_LIB,
	PROJ_TYPE_ELF,
	PROJ_TYPE_BIN,
	PROJ_TYPE_FAT12,

	__PROJ_TYPE_MAX,
} proj_type_t;

// clang-format off
static const str_t s_proj_types[] = {
	[PROJ_TYPE_UNKNOWN] = STRS(""),
	[PROJ_TYPE_LIB]	    = STRS("LIB"),
	[PROJ_TYPE_EXE]	    = STRS("EXE"),
	[PROJ_TYPE_BIN]	    = STRS("BIN"),
	[PROJ_TYPE_ELF]	    = STRS("ELF"),
	[PROJ_TYPE_FAT12]   = STRS("FAT12"),
};
// clang-format on

typedef enum lang_e {
	LANG_NONE,
	LANG_NASM,
	LANG_ASM,
	LANG_C,
	LANG_CPP,

	__LANG_MAX,
} lang_t;

// clang-format off
static const str_t s_langs[] = {
	[LANG_NONE]    = STRS("NONE"),
	[LANG_NASM]    = STRS("NASM"),
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

typedef enum flag_e {
	FLAG_NONE,
	FLAG_STD_C99,
	FLAG_FREESTANDING,
	FLAG_WHOLEARCHIVE,
	FLAG_ALLOWMULTIPLEDEFINITION,
	FLAG_STATIC,
	FLAG_NOSTARTFILES,
	FLAG_MATH,
	FLAG_X11,
	FLAG_GL,
	FLAG_GLX,

	__FLAG_MAX,
} flag_t;

static const str_t s_flags[] = {
	[FLAG_NONE]		       = STRS("NONE"),
	[FLAG_STD_C99]		       = STRS("STD_C99"),
	[FLAG_FREESTANDING]	       = STRS("FREESTANDING"),
	[FLAG_WHOLEARCHIVE]	       = STRS("WHOLEARCHIVE"),
	[FLAG_ALLOWMULTIPLEDEFINITION] = STRS("ALLOWMULTIPLEDEFINITION"),
	[FLAG_STATIC]		       = STRS("STATIC"),
	[FLAG_NOSTARTFILES]	       = STRS("NOSTARTFILES"),
	[FLAG_MATH]		       = STRS("MATH"),
	[FLAG_X11]		       = STRS("X11"),
	[FLAG_GL]		       = STRS("GL"),
	[FLAG_GLX]		       = STRS("GLX"),
};

typedef enum link_type_e {
	LINK_TYPE_NONE,
	LINK_TYPE_STATIC,
	LINK_TYPE_SHARED,
} link_type_t;

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
	arr_t depends;
	arr_t all_depends;
	arr_t includes;
	const struct dir_s *parent;
	pgc_t pgc;
	pgc_t pgcr;
	union {
		make_t make;
		cmake_t cmake;
	} gen;
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
