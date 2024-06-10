#ifndef MK_GEN_H
#define MK_GEN_H

#include "arr.h"
#include "path.h"

#define MK_HEADER_END ARR_END
#define MK_SRC_END    ARR_END

typedef enum mk_gen_header_ext_e {
	MK_EXT_INC,
	MK_EXT_H,
	MK_EXT_HPP,
	__MK_HEADER_MAX,
} mk_gen_header_ext_t;

#define F_MK_EXT_INC (1 << MK_EXT_INC)
#define F_MK_EXT_H   (1 << MK_EXT_H)
#define F_MK_EXT_HPP (1 << MK_EXT_HPP)

typedef enum mk_gen_src_ext_e {
	MK_EXT_ASM,
	MK_EXT_C,
	MK_EXT_CPP,
	__MK_SRC_MAX,
} mk_gen_src_ext_t;

#define F_MK_EXT_ASM (1 << MK_EXT_ASM)
#define F_MK_EXT_C   (1 << MK_EXT_C)
#define F_MK_EXT_CPP (1 << MK_EXT_CPP)

typedef enum mk_gen_intdir_type_e {
	MK_INTDIR_OBJECT,
	MK_INTDIR_STATIC,
	MK_INTDIR_SHARED,
	__MK_INTDIR_MAX,
} mk_gen_intdir_type_t;

#define F_MK_INTDIR_OBJECT (1 << MK_INTDIR_OBJECT)
#define F_MK_INTDIR_STATIC (1 << MK_INTDIR_STATIC)
#define F_MK_INTDIR_SHARED (1 << MK_INTDIR_SHARED)

typedef enum mk_gen_target_type_e {
	MK_TARGET_EXE,
	MK_TARGET_STATIC,
	MK_TARGET_SHARED,
	MK_TARGET_BIN,
	MK_TARGET_ELF,
	MK_TARGET_FAT12,
	__MK_TARGET_MAX,
} mk_gen_target_type_t;

#define F_MK_TARGET_EXE	   (1 << MK_TARGET_EXE)
#define F_MK_TARGET_STATIC (1 << MK_TARGET_STATIC)
#define F_MK_TARGET_SHARED (1 << MK_TARGET_SHARED)
#define F_MK_TARGET_BIN	   (1 << MK_TARGET_BIN)
#define F_MK_TARGET_ELF	   (1 << MK_TARGET_ELF)
#define F_MK_TARGET_FAT12  (1 << MK_TARGET_FAT12)

typedef struct mk_gen_s {
	str_t outdir;
	str_t intdir[__MK_INTDIR_MAX];
	str_t covdir;
	arr_t headers;
	arr_t srcs;
	str_t target[__MK_TARGET_MAX];
	str_t args;
	str_t includes;
	str_t flags[__MK_SRC_MAX];
	str_t defines[__MK_INTDIR_MAX];
	str_t ldflags;
	str_t run[__MK_TARGET_MAX];
	str_t run_debug[__MK_TARGET_MAX];
	str_t artifact[__MK_TARGET_MAX];
} mk_gen_t;

mk_gen_t *mk_gen_init(mk_gen_t *gen);
void mk_gen_free(mk_gen_t *gen);

uint mk_gen_add_header(mk_gen_t *gen, str_t dir, mk_gen_header_ext_t exts);
uint mk_gen_add_src(mk_gen_t *gen, str_t dir, mk_gen_src_ext_t exts);
void mk_gen_add_include(mk_gen_t *gen, str_t dir);
void mk_gen_add_flag(mk_gen_t *gen, str_t flag, mk_gen_src_ext_t exts);
void mk_gen_add_define(mk_gen_t *gen, str_t define, mk_gen_intdir_type_t intdirs);
void mk_gen_add_ldflag(mk_gen_t *gen, str_t ldflag);
void mk_gen_set_run(mk_gen_t *gen, str_t run, mk_gen_target_type_t targets);
void mk_gen_set_run_debug(mk_gen_t *gen, str_t run, mk_gen_target_type_t targets);

int mk_gen_print(const mk_gen_t *gen, print_dst_t dst);

#endif
