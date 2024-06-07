#ifndef MK_GEN_H
#define MK_GEN_H

#include "arr.h"
#include "path.h"

typedef enum mk_gen_flag_e {
	MK_GEN_COVERAGE,
} mk_gen_flag_t;

typedef enum mk_gen_inc_e {
	MK_INC_INC,
	MK_INC_H,
	MK_INC_HPP,
	__MK_INC_MAX,
} mk_gen_inc_t;

typedef enum mk_gen_src_e {
	MK_SRC_ASM_BIN,
	MK_SRC_ASM_OBJ,
	MK_SRC_C,
	MK_SRC_CPP,
	__MK_SRC_MAX,
} mk_gen_src_t;

typedef struct mk_gen_s {
	int flags;
	str_t outdir;
	str_t intdir;
	arr_t incs;
	arr_t srcs;
	str_t target;
	str_t args;
} mk_gen_t;

mk_gen_t *mk_gen_init(mk_gen_t *gen);
void mk_gen_free(mk_gen_t *gen);

uint mk_gen_add_inc(mk_gen_t *gen, str_t dir, mk_gen_inc_t incs);
uint mk_gen_add_src(mk_gen_t *gen, str_t dir, mk_gen_src_t srcs);

int mk_gen_print(const mk_gen_t *gen, print_dst_t dst);

#endif
