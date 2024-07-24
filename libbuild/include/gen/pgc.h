#ifndef PGC_H
#define PGC_H

#include "arr.h"
#include "str.h"

#define PGC_END ARR_END

typedef enum pgc_header_type_e {
	PGC_HEADER_INC,
	PGC_HEADER_H,
	PGC_HEADER_HPP,
	__PGC_HEADER_TYPE_MAX,
} pgc_header_type_t;

#define F_PGC_HEADER_INC (1 << PGC_HEADER_INC)
#define F_PGC_HEADER_H	 (1 << PGC_HEADER_H)
#define F_PGC_HEADER_HPP (1 << PGC_HEADER_HPP)

typedef enum pgc_src_type_e {
	PGC_SRC_NASM,
	PGC_SRC_S,
	PGC_SRC_C,
	PGC_SRC_CPP,
	__PGC_SRC_TYPE_MAX,
} pgc_src_type_t;

#define F_PGC_SRC_NASM (1 << PGC_SRC_NASM)
#define F_PGC_SRC_S    (1 << PGC_SRC_S)
#define F_PGC_SRC_C    (1 << PGC_SRC_C)
#define F_PGC_SRC_CPP  (1 << PGC_SRC_CPP)

typedef enum pgc_file_type_e {
	PGC_FILE_BIN,
	PGC_FILE_ELF,
	__PGC_FILE_TYPE_MAX,
} pgc_file_type_t;

typedef enum pgc_intdir_type_e {
	PGC_INTDIR_OBJECT,
	PGC_INTDIR_STATIC,
	PGC_INTDIR_SHARED,
	__PGC_INTDIR_TYPE_MAX,
} pgc_intdir_type_t;

#define F_PGC_INTDIR_OBJECT (1 << PGC_INTDIR_OBJECT)
#define F_PGC_INTDIR_STATIC (1 << PGC_INTDIR_STATIC)
#define F_PGC_INTDIR_SHARED (1 << PGC_INTDIR_SHARED)

typedef enum pgc_build_type_e {
	PGC_BUILD_EXE,
	PGC_BUILD_STATIC,
	PGC_BUILD_SHARED,
	PGC_BUILD_ELF,
	PGC_BUILD_BIN,
	PGC_BUILD_FAT12,
	__PGC_BUILD_TYPE_MAX,
} pgc_build_type_t;

#define F_PGC_BUILD_EXE	   (1 << PGC_BUILD_EXE)
#define F_PGC_BUILD_STATIC (1 << PGC_BUILD_STATIC)
#define F_PGC_BUILD_SHARED (1 << PGC_BUILD_SHARED)
#define F_PGC_BUILD_ELF	   (1 << PGC_BUILD_ELF)
#define F_PGC_BUILD_BIN	   (1 << PGC_BUILD_BIN)
#define F_PGC_BUILD_FAT12  (1 << PGC_BUILD_FAT12)

typedef enum pgc_link_type_e {
	PGC_LINK_STATIC,
	PGC_LINK_SHARED,
	__PGC_LINK_TYPE_MAX,
} pgc_link_type_t;

typedef enum pgc_lib_type_e {
	PGC_LIB_INT,
	PGC_LIB_EXT,
} pgc_lib_type_t;

typedef enum pgc_str_e {
	PGC_STR_NAME,
	PGC_STR_OUTDIR,
	PGC_STR_COVDIR,
	PGC_STR_ARGS,
	PGC_STR_LDFLAGS,
	PGC_STR_HEADER,
	PGC_STR_CWD,
	PGC_STR_SIZE,
	PGC_STR_URL,
	PGC_STR_FORMAT,
	PGC_STR_CONFIG,
	PGC_STR_TARGETS,
	__PGC_STR_MAX,
} pgc_str_t;

typedef enum pgc_arr_e {
	PGC_ARR_ARCHS,
	PGC_ARR_CONFIGS,
	PGC_ARR_HEADERS,
	PGC_ARR_SRCS,
	PGC_ARR_INCLUDES,
	PGC_ARR_LIBS,
	PGC_ARR_DEPENDS,
	PGC_ARR_FILES,
	PGC_ARR_REQUIRES,
	PGC_ARR_COPYFILES, //TODO: copyfiles for each build type
	__PGC_ARR_MAX,
} pgc_arr_t;

typedef enum pgc_target_str_e {
	PGC_TARGET_STR_RUN,
	PGC_TARGET_STR_RUN_DBG,
	PGC_TARGET_STR_ARTIFACT,
	__PGC_TARGET_STR_MAX,
} pgc_build_str_t;

typedef enum pgc_intdir_str_e {
	PGC_INTDIR_STR_INTDIR,
	PGC_INTDIR_STR_DEFINES,
	__PGC_INTDIR_STR_MAX,
} pgc_intdir_str_t;

typedef enum pgc_src_str_e {
	PGC_SRC_STR_FLAGS,
	__PGC_SRC_STR_MAX,
} pgc_src_str_t;

typedef struct pgc_s {
	str_t str[__PGC_STR_MAX];
	arr_t arr[__PGC_ARR_MAX];
	str_t src[__PGC_SRC_STR_MAX][__PGC_SRC_TYPE_MAX];
	str_t intdir[__PGC_INTDIR_STR_MAX][__PGC_INTDIR_TYPE_MAX];
	str_t target[__PGC_TARGET_STR_MAX][__PGC_BUILD_TYPE_MAX];
	int builds;
} pgc_t;

pgc_t *pgc_init(pgc_t *pgc);
void pgc_free(pgc_t *pgc);

uint pgc_add_arch(pgc_t *pgc, str_t config);
uint pgc_add_config(pgc_t *pgc, str_t config);
uint pgc_add_header(pgc_t *pgc, str_t dir, int exts);
uint pgc_add_src(pgc_t *pgc, str_t dir, int exts);
uint pgc_add_include(pgc_t *pgc, str_t dir);
void pgc_add_flag(pgc_t *pgc, str_t flag, int exts);
void pgc_add_define(pgc_t *pgc, str_t define, int intdirs);
void pgc_add_ldflag(pgc_t *pgc, str_t ldflag);
uint pgc_add_lib(pgc_t *pgc, str_t dir, str_t name, pgc_link_type_t link_type, pgc_lib_type_t lib_type);
uint pgc_add_depend(pgc_t *pgc, str_t depend);
void pgc_set_cwd(pgc_t *pgc, str_t cwd);
void pgc_set_run(pgc_t *pgc, str_t run, int builds);
void pgc_set_run_debug(pgc_t *pgc, str_t run, int builds);
uint pgc_add_file(pgc_t *pgc, str_t path, int ext);
uint pgc_add_require(pgc_t *pgc, str_t require);
uint pgc_add_copyfile(pgc_t *pgc, str_t path);

int pgc_print(const pgc_t *pgc, print_dst_t dst);

#endif
