#ifndef PGC_COMMON_H
#define PGC_COMMON_H

#include "gen/pgc.h"

#include "str.h"

typedef struct pgc_str_flags_s {
	str_t str;
	int flags;
} pgc_str_flags_t;

typedef struct pgc_lib_data_s {
	str_t dir;
	str_t name;
	int intdirs;
	pgc_link_type_t link_type;
	pgc_lib_type_t lib_type;
} pgc_lib_data_t;

typedef struct pgc_depend_data_s {
	str_t name;
	str_t guid;
	str_t rel_dir;
	int builds;
} pgc_depend_data_t;

// clang-format off
static const struct {
	str_t ext;
	pgc_intdir_type_t intdir;
} s_build_c[] = {
	[PGC_BUILD_EXE]	   = { STRS(""),     PGC_INTDIR_OBJECT },
	[PGC_BUILD_STATIC] = { STRS(".a"),   PGC_INTDIR_STATIC },
	[PGC_BUILD_SHARED] = { STRS(".so"),  PGC_INTDIR_SHARED },
	[PGC_BUILD_ELF]	   = { STRS(".elf"), PGC_INTDIR_OBJECT }, 
	[PGC_BUILD_BIN]	   = { STRS(".bin"), PGC_INTDIR_OBJECT }, 
	[PGC_BUILD_FAT12]  = { STRS(".img"), PGC_INTDIR_OBJECT },
};

static const struct {
	str_t postfix;
	str_t folder;
} s_intdir_c[] = {
	[PGC_INTDIR_OBJECT] = { STRS(""),   STRS("") },
	[PGC_INTDIR_STATIC] = { STRS("_s"), STRS("static/") },
	[PGC_INTDIR_SHARED] = { STRS("_d"), STRS("shared/") },
};
// clang-format on

#endif
