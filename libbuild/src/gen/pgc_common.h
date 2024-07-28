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

#endif
