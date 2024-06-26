#ifndef MK_PGEN_H
#define MK_PGEN_H

#include "arr.h"
#include "make.h"
#include "path.h"

#define MK_CONFIG_END  ARR_END
#define MK_HEADER_END  ARR_END
#define MK_SRC_END     ARR_END
#define MK_FILE_END    ARR_END
#define MK_REQUIRE_END ARR_END

typedef enum mk_pgen_header_ext_e {
	MK_EXT_INC,
	MK_EXT_H,
	MK_EXT_HPP,
	__MK_HEADER_MAX,
} mk_pgen_header_ext_t;

#define F_MK_EXT_INC (1 << MK_EXT_INC)
#define F_MK_EXT_H   (1 << MK_EXT_H)
#define F_MK_EXT_HPP (1 << MK_EXT_HPP)

typedef enum mk_pgen_src_ext_e {
	MK_EXT_ASM,
	MK_EXT_S,
	MK_EXT_C,
	MK_EXT_CPP,
	__MK_SRC_MAX,
} mk_pgen_src_ext_t;

#define F_MK_EXT_ASM (1 << MK_EXT_ASM)
#define F_MK_EXT_S   (1 << MK_EXT_S)
#define F_MK_EXT_C   (1 << MK_EXT_C)
#define F_MK_EXT_CPP (1 << MK_EXT_CPP)

typedef enum mk_pgen_file_ext_e {
	MK_EXT_BIN,
	MK_EXT_ELF,
	__MK_EXT_MAX,
} mk_pgen_file_ext_t;

typedef enum mk_pgen_intdir_type_e {
	MK_INTDIR_OBJECT,
	MK_INTDIR_STATIC,
	MK_INTDIR_SHARED,
	__MK_INTDIR_MAX,
} mk_pgen_intdir_type_t;

#define F_MK_INTDIR_OBJECT (1 << MK_INTDIR_OBJECT)
#define F_MK_INTDIR_STATIC (1 << MK_INTDIR_STATIC)
#define F_MK_INTDIR_SHARED (1 << MK_INTDIR_SHARED)

typedef enum mk_pgen_build_type_e {
	MK_BUILD_EXE,
	MK_BUILD_STATIC,
	MK_BUILD_SHARED,
	MK_BUILD_BIN,
	MK_BUILD_ELF,
	MK_BUILD_FAT12,
	__MK_BUILD_MAX,
} mk_pgen_build_type_t;

#define F_MK_BUILD_EXE	  (1 << MK_BUILD_EXE)
#define F_MK_BUILD_STATIC (1 << MK_BUILD_STATIC)
#define F_MK_BUILD_SHARED (1 << MK_BUILD_SHARED)
#define F_MK_BUILD_BIN	  (1 << MK_BUILD_BIN)
#define F_MK_BUILD_ELF	  (1 << MK_BUILD_ELF)
#define F_MK_BUILD_FAT12  (1 << MK_BUILD_FAT12)

typedef struct mk_pgen_s {
	str_t outdir;
	str_t intdir[__MK_INTDIR_MAX];
	str_t covdir;
	arr_t configs;
	arr_t headers;
	arr_t srcs;
	int builds;
	str_t args;
	str_t includes;
	str_t flags[__MK_SRC_MAX];
	str_t defines[__MK_INTDIR_MAX];
	str_t ldflags;
	str_t run[__MK_BUILD_MAX];
	str_t run_debug[__MK_BUILD_MAX];
	str_t artifact[__MK_BUILD_MAX];
	str_t header;
	arr_t files;
	str_t size;
	str_t url;
	str_t name;
	str_t format;
	arr_t requires;
	str_t config;
	str_t targets;
} mk_pgen_t;

mk_pgen_t *mk_pgen_init(mk_pgen_t *gen);
void mk_pgen_free(mk_pgen_t *gen);

uint mk_pgen_add_config(mk_pgen_t *gen, str_t config);
uint mk_pgen_add_header(mk_pgen_t *gen, str_t dir, int exts);
uint mk_pgen_add_src(mk_pgen_t *gen, str_t dir, int exts);
void mk_pgen_add_include(mk_pgen_t *gen, str_t dir);
void mk_pgen_add_flag(mk_pgen_t *gen, str_t flag, int exts);
void mk_pgen_add_define(mk_pgen_t *gen, str_t define, int intdirs);
void mk_pgen_add_ldflag(mk_pgen_t *gen, str_t ldflag);
void mk_pgen_add_slib(mk_pgen_t *gen, str_t lib);
void mk_pgen_add_dlib(mk_pgen_t *gen, str_t lib);
void mk_pgen_add_slib_dir(mk_pgen_t *gen, str_t lib);
void mk_pgen_add_dlib_dir(mk_pgen_t *gen, str_t lib);
void mk_pgen_set_run(mk_pgen_t *gen, str_t run, int builds);
void mk_pgen_set_run_debug(mk_pgen_t *gen, str_t run, int builds);
uint mk_pgen_add_file(mk_pgen_t *gen, str_t path, int ext);
uint mk_pgen_add_require(mk_pgen_t *gen, str_t require);

make_t *mk_pgen(const mk_pgen_t *gen, make_t *make);

#endif
